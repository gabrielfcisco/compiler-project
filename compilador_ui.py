# gui_compiler.py
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

ROOT = tk.Tk()
ROOT.title("Compilador - UI")
ROOT.geometry("1100x760")

# --------------------
# Top controls
# --------------------
top_frame = ttk.Frame(ROOT)
top_frame.pack(fill="x", padx=8, pady=6)

exe_var = tk.StringVar(value="./parser")
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
# Editor area
# --------------------
editor = ScrolledText(ROOT, wrap="none", height=24)
editor.pack(fill="both", expand=True, padx=8, pady=(0,6))

# --------------------
# Notebook for error categories / posfixa / log
# --------------------
notebook = ttk.Notebook(ROOT)
notebook.pack(fill="both", expand=False, padx=8, pady=(0,6), ipady=6)

# Frames for each tab
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

# Helper to create listbox with scrollbar
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

# POSFIXA listbox uses a text preview area below
pos_left = ttk.Frame(frame_pos)
pos_left.pack(side="left", fill="both", expand=True, padx=(0,4), pady=4)
pos_listbox = tk.Listbox(pos_left)
pos_listbox.pack(side="left", fill="both", expand=True)
pos_sb = ttk.Scrollbar(pos_left, orient="vertical", command=pos_listbox.yview)
pos_sb.pack(side="left", fill="y")
pos_listbox.config(yscrollcommand=pos_sb.set)

pos_preview = ScrolledText(frame_pos, height=6, wrap="word")
pos_preview.pack(side="left", fill="both", expand=True, padx=(6,0), pady=4)

# Log (full)
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
        # escreve o log com índice de linhas numerado
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
errors_data_combined = []  # combined list to allow "go to" from a combined view if needed

# temp holder for legacy infix/posfix matching
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
        # try to auto-fix common paste error: trailing ':' (not drive spec)
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
        # normalize newlines -> garante que editor e cálculo de linhas fiquem consistentes
        content = content.replace("\r\n", "\n").replace("\r", "\n")
        editor.delete("1.0", "end")
        editor.insert("1.0", content)
        message_var.set(f"Aberto: {path}")
        append_log(f"Arquivo aberto: {path}")
    except Exception as e:
        tb = traceback.format_exc()
        messagebox.showerror("Erro ao abrir arquivo", f"{e}\n\nVeja o log de erros para mais detalhes.")
        append_log(f"Erro ao abrir arquivo: {e}")
        append_log(tb)
        message_var.set("Erro ao abrir arquivo")

# --------------------
# Regex for classification (adjust if your C output differs slightly)
# --------------------
RE_POSF = re.compile(r"POSFIXA\s+id=(\d+)\s+origem=([^ ]+)\s+infixa=\"([^\"]*)\"\s+posfixa=\"([^\"]*)\"", re.I)
RE_LEX = re.compile(r"\] .*LEXICAL\b.*linha=(\d+).*msg=(.*)", re.I)
# Ajustei nomes tipo para seu report_error: usamos "SYNTACTIC" no error.c
RE_SYN = re.compile(r"\] .*SYNTACTIC\b.*linha=(\d+).*token=([^ ]+).*msg=(.*)", re.I)
RE_SEM = re.compile(r"\] .*SEMANTIC\b.*linha=(\d+).*token=([^ ]+).*msg=(.*)", re.I)
RE_CODE = re.compile(r"\] .*CODEGEN\b.*msg=(.*)", re.I)

# Legacy patterns for "Expressão em notação infixa/posfixa" (captures accented text too)
RE_INF_LEGACY = re.compile(r"notação infixa[:\s]*(.*)", re.I | re.UNICODE)
RE_POS_LEGACY = re.compile(r"notação posfixa[:\s]*(.*)", re.I | re.UNICODE)

# Fallback regex for older formats (keeps compatibility)
FALLBACK_LINE = re.compile(r"ERRO(?: semantico)?:\s*linha\s*(\d+),?.*token:?\s*(.*)", re.I)

# --------------------
# Classification and UI append
# --------------------
def classify_and_append(line):
    """
    Analisa UMA linha de stderr e popula as listas correspondentes.
    Garante que 'linha' seja armazenada como int quando disponível.
    """
    global legacy_infix_temp

    # check POSFIXA structured output first
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

    # Lexical
    m = RE_LEX.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        msg = m.group(2).strip()
        lex_data.append((linha, None, msg, line))
        display_ln = f"L{linha}" if linha else "L?"
        lex_listbox.insert("end", f"{display_ln}: {msg}")
        append_log(f"[LEXICAL] {display_ln}: {msg}")
        return

    # Syntactic
    m = RE_SYN.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        token = m.group(2)
        msg = m.group(3).strip()
        syn_data.append((linha, token, msg, line))
        display_ln = f"L{linha}" if linha else "L?"
        syn_listbox.insert("end", f"{display_ln} {token}: {msg}")
        append_log(f"[SYNTACTIC] {display_ln} {token}: {msg}")
        return

    # Semantic
    m = RE_SEM.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        token = m.group(2)
        msg = m.group(3).strip()
        sem_data.append((linha, token, msg, line))
        display_ln = f"L{linha}" if linha else "L?"
        sem_listbox.insert("end", f"{display_ln} {token}: {msg}")
        append_log(f"[SEMANTIC] {display_ln} {token}: {msg}")
        return

    # Codegen
    m = RE_CODE.search(line)
    if m:
        msg = m.group(1).strip()
        code_data.append((msg, line))
        code_listbox.insert("end", msg)
        append_log(f"[CODEGEN] {msg}")
        return

    # Legacy infixa/posfixa handling (compiler printed them as separate human-readable lines)
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

    # fallback: older ERRO: linha X, token: Y
    m = FALLBACK_LINE.search(line)
    if m:
        try:
            linha = int(m.group(1))
        except Exception:
            linha = None
        token = m.group(2).strip()
        syn_data.append((linha, token, f"token inesperado", line))
        display_ln = f"L{linha}" if linha else "L?"
        syn_listbox.insert("end", f"{display_ln} {token}: token inesperado")
        append_log(f"[SYNTACTIC-FALLBACK] {display_ln} {token}: token inesperado")
        return

    # nothing matched -> just append to log
    append_log(line)

# --------------------
# Click handlers to jump to line in editor
# --------------------
def highlight_line_in_editor(ln):
    try:
        if ln is None:
            return
        # garante que seja inteiro; se não for, tenta extrair dígitos
        try:
            ln_int = int(ln)
        except Exception:
            # tenta extrair um número de dentro de uma string
            s = str(ln)
            m = re.search(r"(\d+)", s)
            if m:
                ln_int = int(m.group(1))
            else:
                return

        # limita o número de linha ao total de linhas do editor
        total_lines = int(editor.index('end-1c').split('.')[0])
        if ln_int < 1:
            ln_int = 1
        if ln_int > total_lines:
            ln_int = total_lines

        start = f"{ln_int}.0"
        end = f"{ln_int}.end"
        editor.tag_remove("hl", "1.0", "end")
        editor.tag_add("hl", start, end)
        editor.tag_config("hl", background="yellow")
        editor.see(start)
    except Exception as e:
        # não parar o programa por conta de highlight; registra no log
        append_log(f"Erro ao destacar linha: {e}")

def on_lex_select(evt):
    sel = lex_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(lex_data): return
    ln = lex_data[idx][0]
    highlight_line_in_editor(ln)

def on_syn_select(evt):
    sel = syn_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(syn_data): return
    ln = syn_data[idx][0]
    highlight_line_in_editor(ln)

def on_sem_select(evt):
    sel = sem_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(sem_data): return
    ln = sem_data[idx][0]
    highlight_line_in_editor(ln)

def on_code_select(evt):
    # Codegen errors may not have line numbers; show raw message in a popup
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
# Compilation worker & processing output
# --------------------
def compile_current():
    path = file_var.get().strip()
    if path.endswith(':') and not (len(path) == 2 and path[1] == ':'):
        candidate = path.rstrip(':').rstrip()
        if os.path.exists(candidate):
            path = candidate
            file_var.set(path)
            append_log("Removido ':' final do caminho automaticamente.")
    if not path or not os.path.exists(path):
        messagebox.showwarning("Arquivo", f"Escolha um arquivo válido antes de compilar.\nAtual: {path}")
        append_log(f"Tentativa de compilar com caminho inválido: {path}")
        return
    compile_btn.config(state="disabled")
    message_var.set("Compilando...")

    def worker():
        out = ""
        err = ""
        try:
            exe = exe_var.get().strip() or "./parser"
            if not os.path.isabs(exe) and shutil.which(exe):
                exe = shutil.which(exe)
            if not os.path.exists(exe):
                possible = os.path.join(os.getcwd(), exe)
                if os.path.exists(possible):
                    exe = possible
            if not os.path.exists(exe):
                raise FileNotFoundError(f"Executável não encontrado: {exe}")

            path_arg = os.path.abspath(path)
            # ensure we decode as utf-8 (compiler should emit utf-8); errors='replace' avoids crashes
            proc = subprocess.Popen([exe, path_arg],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        text=True,
                        encoding='utf-8',
                        errors='replace',
                        cwd=os.path.dirname(exe) or None)
            out, err = proc.communicate(timeout=60)
        except subprocess.TimeoutExpired:
            try:
                proc.kill()
            except Exception:
                pass
            out, err = "", "Tempo esgotado durante compilação."
        except FileNotFoundError as e:
            out, err = "", str(e)
        except Exception as e:
            out, err = "", f"Exceção: {e}\n{traceback.format_exc()}"

        # Update UI
        def ui_update():
            nonlocal out, err
            had_errors = False
            # stdout
            if out.strip():
                append_log("=== stdout ===")
                for l in out.splitlines():
                    append_log(l)
            # stderr: classify line by line
            if err.strip():
                append_log("=== stderr (erros do compilador) ===")
                for l in err.splitlines():
                    # write raw line to full log (to keep originals)
                    error_log.insert("end", l + "\n")
                    classify_and_append(l)
                # decide if any errors were found by checking lists
                if len(lex_data) + len(syn_data) + len(sem_data) + len(code_data) > 0:
                    had_errors = True
                message_var.set("Compilação finalizada com erros.")
            else:
                # no stderr lines -> success
                append_log("Sucesso")
                message_var.set("Compilação finalizada com sucesso.")
            # always enable button
            compile_btn.config(state="normal")
            # ensure log view scrolls
            error_log.see("end")

        ROOT.after(1, ui_update)

    threading.Thread(target=worker, daemon=True).start()

compile_btn.config(command=compile_current)

# --------------------
# Start UI
# --------------------
ROOT.mainloop()
