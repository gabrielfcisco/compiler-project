################################################################################
# Arquivo: compilador_ui.py
# Autores: Enzo, Gabriel, Guilherme, Samuel
# 
#
# Descrição:
#    Implementa a interface gráfica (GUI) do compilador,
#    Responsável por fornecer um editor de código com numeração de linhas,
#    gerenciar a compilação e exibir erros categorizados (léxicos, sintáticos,
#    semânticos e de geração de código), além de visualizar expressões em
#    notação infixa e pós-fixa.
#    
#
# Dependências:
#    - tkinter
#    - subprocess
#    - threading
#    - re (regex)
#    - os
#    - shutil
#    - datetime
#    - tempfile
#    - sys
#    - traceback
#
# Como executar:
#    python3 compilador_ui.py
#
################################################################################

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from tkinter.scrolledtext import ScrolledText
import subprocess
import threading
import re
import os
import shutil
import traceback
from datetime import datetime
import tempfile
import sys

ROOT = tk.Tk()
ROOT.title("Compilador - UI")
ROOT.geometry("1100x760")

# --------------------
# Top controls
# --------------------
top_frame = ttk.Frame(ROOT)
top_frame.pack(fill="x", padx=8, pady=6)

exe_var = tk.StringVar(value="./parser.exe")
ttk.Label(top_frame, text="Executável:").pack(side="left")
exe_entry = ttk.Entry(top_frame, textvariable=exe_var, width=36)
exe_entry.pack(side="left", padx=(4,10))

file_var = tk.StringVar()
ttk.Label(top_frame, text="Arquivo:").pack(side="left")
file_entry = ttk.Entry(top_frame, textvariable=file_var, width=72)
file_entry.pack(side="left", padx=6)

def browse():
    fn = filedialog.askopenfilename(filetypes=[("Texto", "*.txt *.pg *.src *.alg *.c"), ("Todos", "*.*")])
    if fn:
        file_var.set(fn)
        load_file(fn)

ttk.Button(top_frame, text="Abrir", command=browse).pack(side="left", padx=4)
compile_btn = ttk.Button(top_frame, text="Compilar")
compile_btn.pack(side="right", padx=(4,0))

# --------------------
# Editor area with line numbers (gutter)
# --------------------
editor_frame = ttk.Frame(ROOT)
editor_frame.pack(fill="both", expand=True, padx=8, pady=(0,6))

ln_text = tk.Text(editor_frame, width=6, padx=4, takefocus=0, border=0,
                  background="#f0f0f0", state="disabled", wrap="none")
ln_text.pack(side="left", fill="y")

vbar = ttk.Scrollbar(editor_frame, orient="vertical")
vbar.pack(side="right", fill="y")

editor = tk.Text(editor_frame, wrap="none", undo=True)
editor.pack(side="left", fill="both", expand=True)

hbar = ttk.Scrollbar(editor_frame, orient="horizontal", command=editor.xview)
hbar.pack(side="bottom", fill="x")
editor.configure(xscrollcommand=hbar.set)

def on_vscroll(*args):
    try:
        editor.yview(*args)
    except Exception:
        pass
    try:
        ln_text.yview(*args)
    except Exception:
        pass
    try:
        vbar.set(*args)
    except Exception:
        pass

vbar.config(command=on_vscroll)

def on_yscroll(first, last):
    try:
        ln_text.yview_moveto(first)
    except Exception:
        pass
    try:
        vbar.set(first, last)
    except Exception:
        pass

editor.configure(yscrollcommand=on_yscroll)

def _on_mousewheel(event):
    if getattr(event, "num", None) == 4 or getattr(event, "delta", 0) > 0:
        editor.yview_scroll(-1, "units")
    elif getattr(event, "num", None) == 5 or getattr(event, "delta", 0) < 0:
        editor.yview_scroll(1, "units")
    return "break"

editor.bind("<MouseWheel>", _on_mousewheel)
editor.bind("<Button-4>", _on_mousewheel)
editor.bind("<Button-5>", _on_mousewheel)

# --------------------
# Notebook for error categories / posfixa / log
# --------------------
notebook = ttk.Notebook(ROOT)
notebook.pack(fill="both", expand=False, padx=8, pady=(0,6), ipady=6)

frame_lex = ttk.Frame(notebook)
frame_syn = ttk.Frame(notebook)
frame_sem = ttk.Frame(notebook)
frame_code = ttk.Frame(notebook)
frame_pos = ttk.Frame(notebook)
frame_log = ttk.Frame(notebook)

notebook.add(frame_lex, text="Lexical")
notebook.add(frame_syn, text="Sintático")
notebook.add(frame_sem, text="Semântico")
notebook.add(frame_code, text="Geração Código")
notebook.add(frame_pos, text="POSFIXA")
notebook.add(frame_log, text="Log Completo")

# default to Log Completo tab
try:
    notebook.select(frame_log)
except Exception:
    pass

def make_listbox(frame):
    lb = tk.Listbox(frame)
    lb.pack(side="left", fill="both", expand=True, padx=(0,4), pady=4)
    sb = ttk.Scrollbar(frame, orient="vertical", command=lb.yview)
    sb.pack(side="left", fill="y", pady=4)
    lb.config(yscrollcommand=sb.set)
    return lb

lex_listbox = make_listbox(frame_lex)
syn_listbox = make_listbox(frame_syn)
sem_listbox = make_listbox(frame_sem)
code_listbox = make_listbox(frame_code)

pos_left = ttk.Frame(frame_pos)
pos_left.pack(side="left", fill="both", expand=True, padx=(0,4), pady=4)
pos_listbox = tk.Listbox(pos_left)
pos_listbox.pack(side="left", fill="both", expand=True)
pos_sb = ttk.Scrollbar(pos_left, orient="vertical", command=pos_listbox.yview)
pos_sb.pack(side="left", fill="y")
pos_listbox.config(yscrollcommand=pos_sb.set)

pos_preview = ScrolledText(frame_pos, height=6, wrap="word")
pos_preview.pack(side="left", fill="both", expand=True, padx=(6,0), pady=4)

error_log = ScrolledText(frame_log, wrap="none", height=10)
error_log.pack(fill="both", expand=True, padx=4, pady=4)

# --------------------
# Buttons under logs: Save / Clear
# --------------------
log_btn_frame = ttk.Frame(ROOT)
log_btn_frame.pack(fill="x", padx=8, pady=(0,6))

def save_log():
    fn = filedialog.asksaveasfilename(defaultextension=".log",
                                      filetypes=[("Log", "*.log"),("Text", "*.txt"),("All","*.*")])
    if not fn:
        return
    try:
        content = error_log.get("1.0", "end").rstrip("\n")
        lines = content.splitlines()
        with open(fn, "w", encoding="utf-8") as f:
            for i, ln in enumerate(lines, start=1):
                f.write(f"{i:04d}: {ln}\n")
        messagebox.showinfo("Salvar Log", f"Log salvo em: {fn}")
    except Exception as e:
        messagebox.showerror("Erro", f"Não foi possível salvar o log: {e}")

def clear_log():
    if not messagebox.askyesno("Limpar Log", "Tem certeza que deseja limpar o log de erros?"):
        return
    error_log.delete("1.0", "end")
    lex_listbox.delete(0, "end")
    syn_listbox.delete(0, "end")
    sem_listbox.delete(0, "end")
    code_listbox.delete(0, "end")
    pos_listbox.delete(0, "end")
    pos_preview.delete("1.0", "end")
    global lex_data, syn_data, sem_data, code_data, pos_data
    lex_data = []
    syn_data = []
    sem_data = []
    code_data = []
    pos_data = []
    global errors_data_combined
    errors_data_combined = []

ttk.Button(log_btn_frame, text="Salvar Log", command=save_log).pack(side="right", padx=4)
ttk.Button(log_btn_frame, text="Limpar Log", command=clear_log).pack(side="right")

# --------------------
# Status bar
# --------------------
message_var = tk.StringVar()
status = ttk.Label(ROOT, textvariable=message_var, relief="sunken", anchor="w")
status.pack(fill="x", side="bottom", pady=(6,0))

# --------------------
# Data containers
# --------------------
lex_data = []   # list of tuples (linha, token, msg, raw)
syn_data = []   # list of tuples (linha, token, msg, raw)
sem_data = []   # list of tuples (linha, token, msg, raw)
code_data = []  # list of tuples (msg, raw)
pos_data = []   # list of tuples (id, origem, infixa, posfixa, raw)
errors_data_combined = []

legacy_infix_temp = None

# --------------------
# Logging helpers
# --------------------
def append_log(text):
    ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    error_log.insert("end", f"[{ts}] {text}\n")
    error_log.see("end")

# --------------------
# File load helper
# --------------------
def load_file(path):
    try:
        if not path:
            raise FileNotFoundError("Caminho vazio — verifique o campo 'Arquivo'.")
        if path.endswith(':') and not (len(path) == 2 and path[1] == ':'):
            candidate = path.rstrip(':').rstrip()
            if os.path.exists(candidate):
                path = candidate
                file_var.set(path)
                append_log(f"Ajustado caminho removendo ':' final: {path}")
        path = os.path.abspath(path)
        if not os.path.exists(path):
            raise FileNotFoundError(f"Arquivo não encontrado: {path}")
        if os.path.isdir(path):
            raise IsADirectoryError(f"O caminho é um diretório, não um arquivo: {path}")
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
        content = content.replace("\r\n", "\n").replace("\r", "\n")
        editor.delete("1.0", "end")
        editor.insert("1.0", content)
        update_line_numbers()
        message_var.set(f"Aberto: {path}")
        append_log(f"Arquivo aberto: {path}")
    except Exception as e:
        tb = traceback.format_exc()
        messagebox.showerror("Erro ao abrir arquivo", f"{e}\n\nVeja o log de erros para mais detalhes.")
        append_log(f"Erro ao abrir arquivo: {e}")
        append_log(tb)
        message_var.set("Erro ao abrir arquivo")

# --------------------
# Regex for classification
# --------------------
RE_POSF = re.compile(r"POSFIXA\s+id=(\d+)\s+origem=([^ ]+)\s+infixa=\"([^\"]*)\"\s+posfixa=\"([^\"]*)\"", re.I)
RE_LEX = re.compile(r"\] .*LEXICAL\b.*linha=(\d+).*msg=(.*)", re.I)
RE_SYN = re.compile(r"\] .*SYNTACTIC\b.*linha=(\d+).*token=([^ ]+).*msg=(.*)", re.I)
RE_SEM = re.compile(r"\] .*SEMANTIC\b.*linha=(\d+).*token=([^ ]+).*msg=(.*)", re.I)
RE_CODE = re.compile(r"\] .*CODEGEN\b.*msg=(.*)", re.I)

RE_INF_LEGACY = re.compile(r"notação infixa[:\s]*(.*)", re.I | re.UNICODE)
RE_POS_LEGACY = re.compile(r"notação posfixa[:\s]*(.*)", re.I | re.UNICODE)

FALLBACK_LINE = re.compile(r"ERRO(?: semantico)?:\s*linha\s*(\d+),?.*token:?\s*(.*)", re.I)

# --------------------
# Line numbers (gutter) updating
# --------------------
def update_line_numbers():
    last = editor.index("end-1c").split('.')[0]
    lines = int(last)
    nums = "\n".join(f"{i}." for i in range(1, lines+1))
    ln_text.config(state="normal")
    ln_text.delete("1.0", "end")
    ln_text.insert("1.0", nums + ("\n" if nums else ""))
    ln_text.config(state="disabled")

def update_line_numbers_async(event=None):
    ROOT.after(10, update_line_numbers)

def on_editor_modified(event=None):
    try:
        if editor.edit_modified():
            update_line_numbers()
            editor.edit_modified(False)
    except Exception:
        pass

editor.bind("<<Modified>>", on_editor_modified)
editor.bind("<KeyRelease>", update_line_numbers_async)
editor.bind("<ButtonRelease>", update_line_numbers_async)
editor.bind("<Configure>", update_line_numbers_async)

# --------------------
# Small helper: procura token no editor e retorna linha (1-based) ou None
# --------------------
def find_line_for_token(token):
    if not token:
        return None
    content = editor.get("1.0", "end-1c")
    try:
        regex_word = re.compile(r"\b" + re.escape(token) + r"\b")
        m = regex_word.search(content)
        if m:
            start_idx = m.start()
            idx = editor.index(f"1.0+{start_idx}c")
            return int(idx.split('.')[0])
    except Exception:
        pass
    try:
        pos = editor.search(token, "1.0", "end")
        if pos:
            return int(pos.split('.')[0])
    except Exception:
        pass
    return None

def resolve_line_number(token, ln_hint):
    try:
        if ln_hint is not None:
            try:
                ln_int = int(ln_hint)
            except Exception:
                ln_int = None
            if ln_int is not None:
                total = int(editor.index('end-1c').split('.')[0])
                if ln_int < 1:
                    ln_int = 1
                if ln_int > total:
                    ln_int = total
                try:
                    line_text = editor.get(f"{ln_int}.0", f"{ln_int}.end")
                except Exception:
                    line_text = ""
                if token and (token in line_text or re.search(r"\b" + re.escape(token) + r"\b", line_text)):
                    return ln_int
        return find_line_for_token(token)
    except Exception as e:
        append_log(f"Erro em resolve_line_number: {e}")
        return None

# --------------------
# Classification and UI append
# --------------------
def classify_and_append(line):
    global legacy_infix_temp

    m = RE_POSF.search(line)
    if m:
        try:
            pid = int(m.group(1))
        except Exception:
            pid = m.group(1)
        origem = m.group(2)
        infixa = m.group(3)
        posfixa = m.group(4)
        pos_data.append((pid, origem, infixa, posfixa, line))
        pos_listbox.insert("end", f"id={pid} orig={origem} -> {infixa} => {posfixa}")
        append_log(f"[POSFIXA] id={pid} origem={origem} in=\"{infixa}\" pos=\"{posfixa}\"")
        return

    m = RE_LEX.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        msg = m.group(2).strip()
        resolved = linha
        lex_data.append((resolved, None, msg, line))
        display_ln = f"Linha  {resolved}" if resolved else "Linha  ?"
        lex_listbox.insert("end", f"{display_ln}: {msg}")
        if resolved:
            append_log(f"[LEXICAL] Linha  {resolved}: {msg}")
        else:
            append_log(f"[LEXICAL] {msg}")
        return

    m = RE_SYN.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        token = m.group(2)
        msg = m.group(3).strip()
        resolved = resolve_line_number(token, linha)
        if resolved:
            syn_data.append((resolved, token, msg, line))
            syn_listbox.insert("end", f"Linha  {resolved} {token}: {msg}")
            append_log(f"[SYNTACTIC] Linha  {resolved} {token}: {msg}")
        else:
            syn_data.append((linha, token, msg, line))
            display_ln = f"Linha  {linha}" if linha else "Linha  ?"
            syn_listbox.insert("end", f"{display_ln} {token}: {msg}")
            append_log(f"[SYNTACTIC] {display_ln} {token}: {msg}")
        return

    m = RE_SEM.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        token = m.group(2)
        msg = m.group(3).strip()
        resolved = resolve_line_number(token, linha)
        if resolved:
            sem_data.append((resolved, token, msg, line))
            sem_listbox.insert("end", f"Linha  {resolved} {token}: {msg}")
            append_log(f"[SEMANTIC] Linha  {resolved} {token}: {msg}")
        else:
            sem_data.append((linha, token, msg, line))
            display_ln = f"Linha  {linha}" if linha else "Linha  ?"
            sem_listbox.insert("end", f"{display_ln} {token}: {msg}")
            append_log(f"[SEMANTIC] {display_ln} {token}: {msg}")
        return

    m = RE_CODE.search(line)
    if m:
        msg = m.group(1).strip()
        code_data.append((msg, line))
        code_listbox.insert("end", msg)
        append_log(f"[CODEGEN] {msg}")
        return

    m = RE_INF_LEGACY.search(line)
    if m:
        inf = m.group(1).strip()
        legacy_infix_temp = inf
        append_log(f"[LEGACY-INFIX] {inf}")
        return

    m = RE_POS_LEGACY.search(line)
    if m:
        pos = m.group(1).strip()
        inf = legacy_infix_temp
        pid = f"LEG{len(pos_data)+1}"
        pos_data.append((pid, "legacy", inf if inf else "", pos, line))
        pos_listbox.insert("end", f"id={pid} orig=legacy -> {inf if inf else '(infixa ausente)'} => {pos}")
        append_log(f"[POSFIXA-LEGACY] id={pid} in=\"{inf}\" pos=\"{pos}\"")
        legacy_infix_temp = None
        return

    m = FALLBACK_LINE.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        token = m.group(2).strip()
        resolved = resolve_line_number(token, linha)
        if resolved:
            syn_data.append((resolved, token, "token inesperado", line))
            syn_listbox.insert("end", f"Linha  {resolved} {token}: token inesperado")
            append_log(f"[SYNTACTIC-FALLBACK] Linha  {resolved} {token}: token inesperado")
        else:
            syn_data.append((linha, token, "token inesperado", line))
            display_ln = f"Linha  {linha}" if linha else "Linha  ?"
            syn_listbox.insert("end", f"{display_ln} {token}: token inesperado")
            append_log(f"[SYNTACTIC-FALLBACK] {display_ln} {token}: token inesperado")
        return

    append_log(line)

# --------------------
# Click handlers / highlighting
# --------------------
def update_listbox_entry(listbox, index, text):
    sel = listbox.curselection()
    was_selected = (sel and sel[0] == index)
    listbox.delete(index)
    listbox.insert(index, text)
    if was_selected:
        listbox.selection_set(index)

def highlight_line_in_editor(ln, token=None):
    try:
        if ln is None and not token:
            return None
        ln_int = None
        if ln is not None:
            try:
                ln_int = int(ln)
            except Exception:
                m = re.search(r"(\d+)", str(ln))
                if m:
                    ln_int = int(m.group(1))
        total_lines = int(editor.index('end-1c').split('.')[0])
        if ln_int is not None:
            if ln_int < 1:
                ln_int = 1
            if ln_int > total_lines:
                ln_int = total_lines
        def do_highlight(line_number):
            start = f"{line_number}.0"
            end = f"{line_number}.end"
            editor.tag_remove("hl", "1.0", "end")
            editor.tag_add("hl", start, end)
            editor.tag_config("hl", background="yellow")
            editor.see(start)
        if ln_int is not None:
            if token:
                try:
                    line_text = editor.get(f"{ln_int}.0", f"{ln_int}.end")
                except Exception:
                    line_text = ""
                if token and (token in line_text or re.search(r"\b" + re.escape(token) + r"\b", line_text)):
                    do_highlight(ln_int)
                    return ln_int
                found_line = find_line_for_token(token)
                if found_line:
                    do_highlight(found_line)
                    return found_line
                do_highlight(ln_int)
                return ln_int
            else:
                do_highlight(ln_int)
                return ln_int
        if token:
            found_line = find_line_for_token(token)
            if found_line:
                do_highlight(found_line)
                return found_line
        return None
    except Exception as e:
        append_log(f"Erro ao destacar linha: {e}")
        return None

def on_lex_select(evt):
    sel = lex_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(lex_data): return
    ln = lex_data[idx][0]
    highlight_line_in_editor(ln, token=None)

def on_syn_select(evt):
    sel = syn_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(syn_data): return
    origem_tuple = syn_data[idx]
    ln, token, msg, raw = origem_tuple[0], origem_tuple[1], origem_tuple[2], origem_tuple[3]
    resolved = highlight_line_in_editor(ln, token=token)
    if resolved and (ln is None or resolved != ln):
        syn_data[idx] = (resolved, token, msg, raw)
        new_text = f"Linha  {resolved} {token}: {msg}"
        update_listbox_entry(syn_listbox, idx, new_text)

def on_sem_select(evt):
    sel = sem_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(sem_data): return
    origem_tuple = sem_data[idx]
    ln, token, msg, raw = origem_tuple[0], origem_tuple[1], origem_tuple[2], origem_tuple[3]
    resolved = highlight_line_in_editor(ln, token=token)
    if resolved and (ln is None or resolved != ln):
        sem_data[idx] = (resolved, token, msg, raw)
        new_text = f"Linha  {resolved} {token}: {msg}"
        update_listbox_entry(sem_listbox, idx, new_text)

def on_code_select(evt):
    sel = code_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(code_data): return
    msg = code_data[idx][0]
    messagebox.showinfo("Erro de Geração de Código", msg)

def on_pos_select(evt):
    sel = pos_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(pos_data): return
    pid, origem, infixa, posfixa, raw = pos_data[idx]
    pos_preview.delete("1.0", "end")
    pos_preview.insert("end", f"ID: {pid}\nOrigem: {origem}\n\nInfixa:\n{infixa}\n\nPosfixa:\n{posfixa}\n")
    pos_preview.see("1.0")

lex_listbox.bind("<<ListboxSelect>>", on_lex_select)
syn_listbox.bind("<<ListboxSelect>>", on_syn_select)
sem_listbox.bind("<<ListboxSelect>>", on_sem_select)
code_listbox.bind("<<ListboxSelect>>", on_code_select)
pos_listbox.bind("<<ListboxSelect>>", on_pos_select)

# --------------------
# File opener and clickable-link helper
# --------------------
def open_file_with_default(path):
    try:
        if not path:
            return
        if os.name == 'nt':
            os.startfile(path)
        elif sys.platform == 'darwin':
            subprocess.Popen(['open', path])
        else:
            subprocess.Popen(['xdg-open', path])
    except Exception as e:
        messagebox.showerror("Abrir arquivo", f"Não foi possível abrir o arquivo:\n{e}")

# counter for unique tags
_link_counter = 0
def insert_clickable_link(file_path, link_text="./output/codigo_maquina_virtual.obj", prefix_text="Código assembly gerado em "):
    global _link_counter
    if not os.path.exists(file_path):
        append_log(f"Arquivo de assembly não encontrado: {file_path}")
        return
    tag = f"link{_link_counter}"
    _link_counter += 1
    # insert prefix then the clickable link text (with tag)
    error_log.insert("end", prefix_text)
    error_log.insert("end", link_text + "\n", (tag,))
    error_log.tag_config(tag, foreground="blue", underline=True)
    # bind click to open file
    def _open(event, p=file_path):
        open_file_with_default(p)
    # text widget tag_bind expects function with one arg
    error_log.tag_bind(tag, "<Button-1>", _open)
    error_log.see("end")

# --------------------
# Compilation worker & processing output (improved detection of actual errors)
# --------------------
def compile_current():
    path_field = file_var.get().strip()
    if path_field.endswith(':') and not (len(path_field) == 2 and path_field[1] == ':'):
        candidate = path_field.rstrip(':').rstrip()
        if os.path.exists(candidate):
            file_var.set(candidate)
            append_log("Removido ':' final do caminho automaticamente.")
    compile_btn.config(state="disabled")
    message_var.set("Compilando...")

    def worker():
        out = ""
        err = ""
        rc = None
        tmp_path = None
        try:
            exe = exe_var.get().strip() or "./parser.exe"
            if not os.path.isabs(exe) and shutil.which(exe):
                exe = shutil.which(exe)
            if not os.path.exists(exe):
                possible = os.path.join(os.getcwd(), exe)
                if os.path.exists(possible):
                    exe = possible
            if not os.path.exists(exe):
                raise FileNotFoundError(f"Executável não encontrado: {exe}")

            content = editor.get("1.0", "end-1c")
            content = content.replace("\r\n", "\n").replace("\r", "\n")
            tmp = tempfile.NamedTemporaryFile(delete=False, suffix=".pg", mode="w", encoding="utf-8")
            tmp.write(content)
            tmp.close()
            tmp_path = tmp.name

            path_arg = os.path.abspath(tmp_path)
            proc = subprocess.Popen([exe, path_arg],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        text=True,
                        encoding='utf-8',
                        errors='replace',
                        cwd=os.path.dirname(exe) or None)
            out, err = proc.communicate(timeout=60)
            rc = proc.returncode
        except subprocess.TimeoutExpired:
            try:
                proc.kill()
            except Exception:
                pass
            out, err = "", "Tempo esgotado durante compilação."
            rc = -1
        except FileNotFoundError as e:
            out, err = "", str(e)
            rc = -1
        except Exception as e:
            out, err = "", f"Exceção: {e}\n{traceback.format_exc()}"
            rc = -1
        finally:
            try:
                if tmp_path and os.path.exists(tmp_path):
                    os.remove(tmp_path)
            except Exception:
                pass

        def ui_update():
            nonlocal out, err, rc
            if out.strip():
                append_log("=== stdout ===")
                for l in out.splitlines():
                    append_log(l)

            if err.strip():
                append_log("=== stderr (erros do compilador) ===")
                for l in err.splitlines():
                    classify_and_append(l)

            total_errors_now = len(lex_data) + len(syn_data) + len(sem_data) + len(code_data)

            # detect meaningful stderr errors (ignore mere info lines)
            error_indicators = re.compile(r'\b(ERRO|Erro|erro|token inesperado|LEXICAL|SYNTACTIC|SEMANTIC|CODEGEN)\b', re.I)
            had_stderr_errors = False
            if err.strip():
                for l in err.splitlines():
                    if error_indicators.search(l):
                        had_stderr_errors = True
                        break

            had_errors = False
            if rc is not None and rc != 0:
                had_errors = True
                append_log(f"[PROCESS RETURN CODE] returncode={rc}")
            if total_errors_now > 0:
                had_errors = True
            if had_stderr_errors:
                had_errors = True

            # Decide success vs error
            if not had_errors:
                append_log("Sucesso")
                message_var.set("Sucesso")
                # try to find the generated assembly file and insert clickable link
                # common filename used by seu gerador: 'codigo_maquina_virtual.obj'
                exe = exe_var.get().strip() or "./parser.exe"
                search_dirs = []
                if os.path.isabs(exe) and os.path.exists(exe):
                    search_dirs.append(os.path.dirname(exe))
                search_dirs.append(os.getcwd())
                # also check temp and the file path of last used file dir (if provided)
                provided_path = file_var.get().strip()
                if provided_path:
                    search_dirs.append(os.path.dirname(os.path.abspath(provided_path)))
                found = None
                target_name = "./output/codigo_maquina_virtual.obj"
                for d in search_dirs:
                    if not d:
                        continue
                    candidate = os.path.join(d, target_name)
                    if os.path.exists(candidate):
                        found = candidate
                        break
                # if not found, also try recursive search up to current dir (fast)
                if not found:
                    for root, dirs, files in os.walk(os.getcwd()):
                        if target_name in files:
                            found = os.path.join(root, target_name)
                            break
                if found:
                    append_log(f"Arquivo de código de máquina encontrado: {found}")
                    # insert clickable LINK into the log
                    insert_clickable_link(found, link_text="./output/codigo_maquina_virtual.obj", prefix_text="Código assembly gerado em ")
                else:
                    append_log(f"Atenção: compilação teve sucesso mas arquivo '{target_name}' não encontrado.")
            else:
                append_log(f"Compilação finalizada com erros. (exit={rc}, itens classificados={total_errors_now})")
                message_var.set("Compilação finalizada com erros.")

            compile_btn.config(state="normal")
            error_log.see("end")
            update_line_numbers()

        ROOT.after(1, ui_update)

    threading.Thread(target=worker, daemon=True).start()

compile_btn.config(command=compile_current)

# --------------------
# Start UI
# --------------------
update_line_numbers()
ROOT.mainloop()
