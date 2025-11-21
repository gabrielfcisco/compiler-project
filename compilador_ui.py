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
ROOT.geometry("1000x720")

# Top: executável + file open + compile button
top_frame = ttk.Frame(ROOT)
top_frame.pack(fill="x", padx=8, pady=6)

exe_var = tk.StringVar(value="./parser")
ttk.Label(top_frame, text="Executável:").pack(side="left")
exe_entry = ttk.Entry(top_frame, textvariable=exe_var, width=30)
exe_entry.pack(side="left", padx=(4,10))

file_var = tk.StringVar()
ttk.Label(top_frame, text="Arquivo:").pack(side="left")
file_entry = ttk.Entry(top_frame, textvariable=file_var, width=60)
file_entry.pack(side="left", padx=6)

def browse():
    fn = filedialog.askopenfilename(filetypes=[("Texto", "*.txt *.pg *.src *.alg *.c"), ("Todos", "*.*")])
    if fn:
        file_var.set(fn)
        load_file(fn)

ttk.Button(top_frame, text="Abrir", command=browse).pack(side="left", padx=4)
compile_btn = ttk.Button(top_frame, text="Compilar")
compile_btn.pack(side="right", padx=(4,0))

# Middle: editor
editor = ScrolledText(ROOT, wrap="none", height=22)
editor.pack(fill="both", expand=True, padx=8, pady=(0,6))

# Bottom: errors list + error log (side-by-side)
bottom = ttk.Frame(ROOT)
bottom.pack(fill="both", expand=False, padx=8, pady=6)

# Left: parsed errors list
err_frame = ttk.LabelFrame(bottom, text="Erros (clicável)")
err_frame.pack(side="left", fill="both", expand=True, padx=(0,6))

errors_listbox = tk.Listbox(err_frame, height=10)
errors_listbox.pack(side="left", fill="both", expand=True)
errors_scroll = ttk.Scrollbar(err_frame, orient="vertical", command=errors_listbox.yview)
errors_scroll.pack(side="right", fill="y")
errors_listbox.config(yscrollcommand=errors_scroll.set)

# Right: accumulated error log (texto)
log_frame = ttk.LabelFrame(bottom, text="Log de Erros (acumulado)")
log_frame.pack(side="left", fill="both", expand=True)

error_log = ScrolledText(log_frame, wrap="none", height=10)
error_log.pack(fill="both", expand=True)

# Buttons under logs: Save / Clear
log_btn_frame = ttk.Frame(ROOT)
log_btn_frame.pack(fill="x", padx=8)
def save_log():
    fn = filedialog.asksaveasfilename(defaultextension=".log", filetypes=[("Log", "*.log"),("Text", "*.txt"),("All","*.*")])
    if not fn:
        return
    try:
        content = error_log.get("1.0", "end")
        with open(fn, "w", encoding="utf-8") as f:
            f.write(content)
        messagebox.showinfo("Salvar Log", f"Log salvo em: {fn}")
    except Exception as e:
        messagebox.showerror("Erro", f"Não foi possível salvar o log: {e}")

def clear_log():
    if not messagebox.askyesno("Limpar Log", "Tem certeza que deseja limpar o log de erros?"):
        return
    error_log.delete("1.0", "end")
    errors_listbox.delete(0, "end")
    global errors_data
    errors_data = []

ttk.Button(log_btn_frame, text="Salvar Log", command=save_log).pack(side="right", padx=4)
ttk.Button(log_btn_frame, text="Limpar Log", command=clear_log).pack(side="right")

# simple status / message box
message_var = tk.StringVar()
status = ttk.Label(ROOT, textvariable=message_var, relief="sunken", anchor="w")
status.pack(fill="x", side="bottom", pady=(6,0))

# helper functions
def append_log(text):
    ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    error_log.insert("end", f"[{ts}] {text}\n")
    error_log.see("end")

# improved load_file with detailed logging and small auto-correction
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
        # read with replace to avoid UnicodeDecodeError crash
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
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

# parse error lines using regex (customize de acordo com saída do seu compilador)
ERR_RE = re.compile(r"(?:linha|line)\s*(\d+)[^\d]*(?:col|coluna)?\s*(\d+)?[:\-\s]*(.*)", re.I)

# global list holding tuples for clickable errors
errors_data = []

def append_parsed_errors(err_text):
    """
    Parse errors and append to errors_listbox + errors_data (do NOT clear existing).
    Returns number of new entries appended.
    """
    lines = err_text.strip().splitlines()
    added = 0
    for l in lines:
        m = ERR_RE.search(l)
        if m:
            ln = int(m.group(1))
            col = int(m.group(2)) if m.group(2) else None
            msg = m.group(3).strip()
            display = f"L{ln}" + (f",C{col}: " if col else ": ") + msg
            errors_listbox.insert("end", display)
            errors_data.append((ln, col, msg, l))
            added += 1
        else:
            # fallback: show raw as non-clickable (store None)
            errors_listbox.insert("end", l)
            errors_data.append((None, None, l, l))
            added += 1
    return added

# when selecting an error, highlight the line in the editor
def on_error_select(evt):
    sel = errors_listbox.curselection()
    if not sel:
        return
    idx = sel[0]
    if idx >= len(errors_data):
        return
    data = errors_data[idx]
    ln = data[0]
    if ln:
        start = f"{ln}.0"
        end = f"{ln}.end"
        editor.tag_remove("hl", "1.0", "end")
        editor.tag_add("hl", start, end)
        editor.tag_config("hl", background="yellow")
        editor.see(start)

errors_listbox.bind("<<ListboxSelect>>", on_error_select)

# compile using subprocess in thread to avoid blocking GUI
def compile_current():
    path = file_var.get().strip()
    # small normalization: remove accidental trailing colon from pasted path
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
            proc = subprocess.Popen([exe, path_arg],
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    text=True,
                                    cwd=os.path.dirname(exe) or None)
            out, err = proc.communicate(timeout=30)
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

        # (segue o código que atualiza a UI com out/err — o mesmo que você já tem)

        # atualizar UI no thread principal
        def ui_update():
            nonlocal out, err
            # append stdout (se houver) e stderr no log acumulado
            if out.strip():
                append_log("=== stdout ===")
                for l in out.splitlines():
                    append_log(l)
                message_var.set("Compilação finalizada. Houveram mensagens em stdout.")
            if err.strip():
                append_log("=== stderr (erros do compilador) ===")
                for l in err.splitlines():
                    append_log(l)
                # parseia e adiciona às entradas parseadas (sem apagar o que já existe)
                try:
                    append_parsed_errors(err)
                except Exception:
                    append_log("Erro ao parsear erros: " + traceback.format_exc())
                message_var.set("Compilação finalizada com erros.")
            if not out.strip() and not err.strip():
                append_log("Sem saída (stdout/stderr).")
                message_var.set("Compilação finalizada. Sem saída.")
            compile_btn.config(state="normal")
        ROOT.after(1, ui_update)

    threading.Thread(target=worker, daemon=True).start()

compile_btn.config(command=compile_current)

# start
ROOT.mainloop()
