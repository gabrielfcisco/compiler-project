import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from mvd_machine import MVDMachine


class MVDApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Máquina Virtual Didática")

        # Estado da MVD
        self.vm = None
        self.current_program_text = ""

        # Interface
        self._build_menu()
        self._build_widgets()
        self.update_buttons()

    # =========================
    # Menu
    # =========================

    def _build_menu(self):
        menubar = tk.Menu(self.root)

        menu_arquivo = tk.Menu(menubar, tearoff=0)
        menu_arquivo.add_command(label="Abrir .obj", command=self.load_obj_file)
        menu_arquivo.add_separator()
        menu_arquivo.add_command(label="Sair", command=self.root.quit)
        menubar.add_cascade(label="Arquivo", menu=menu_arquivo)

        menu_sobre = tk.Menu(menubar, tearoff=0)
        menu_sobre.add_command(
            label="Sobre",
            command=lambda: messagebox.showinfo(
                "Sobre",
                "Máquina Virtual Didática\nSimulador em Python/Tkinter."
            )
        )
        menubar.add_cascade(label="Sobre", menu=menu_sobre)

        self.root.config(menu=menubar)

    # =========================
    # Layout Principal
    # =========================

    def _build_widgets(self):
        # Código de Máquina
        frame_code = ttk.LabelFrame(self.root, text="Código de Máquina")
        # janela maior: aumenta área de código
        frame_code.place(x=10, y=10, width=700, height=450)

        columns = ("linha", "instr", "a1", "a2", "label")
        self.tree_code = ttk.Treeview(frame_code, columns=columns, show="headings")
        self.tree_code.heading("linha", text="Linha")
        self.tree_code.heading("instr", text="Instrução")
        self.tree_code.heading("a1", text="Atributo 1")
        self.tree_code.heading("a2", text="Atributo 2")
        self.tree_code.heading("label", text="Rótulo")

        self.tree_code.column("linha", width=60, anchor="center")
        self.tree_code.column("instr", width=120, anchor="center")
        self.tree_code.column("a1", width=120, anchor="center")
        self.tree_code.column("a2", width=120, anchor="center")
        self.tree_code.column("label", width=120, anchor="center")

        self.tree_code.pack(fill="both", expand=True)

        # Memória (Pilha)
        frame_mem = ttk.LabelFrame(self.root, text="Memória (Pilha)")
        frame_mem.place(x=720, y=10, width=360, height=450)

        self.tree_mem = ttk.Treeview(frame_mem, columns=("end", "val"), show="headings")
        self.tree_mem.heading("end", text="Endereço")
        self.tree_mem.heading("val", text="Valor")
        self.tree_mem.column("end", width=100, anchor="center")
        self.tree_mem.column("val", width=200, anchor="center")
        self.tree_mem.pack(fill="both", expand=True)

        # Saída de Dados
        frame_out = ttk.LabelFrame(self.root, text="Saída de Dados")
        frame_out.place(x=10, y=470, width=550, height=160)

        # altura maior + fonte monoespaçada
        self.txt_out = tk.Text(frame_out, height=6, font=("Consolas", 11))
        self.txt_out.pack(fill="both", expand=True)

        # Modo de Execução
        frame_mode = ttk.LabelFrame(self.root, text="Modo de Execução")
        frame_mode.place(x=580, y=470, width=220, height=160)

        self.exec_mode = tk.StringVar(value="normal")

        rb_normal = ttk.Radiobutton(
            frame_mode, text="Normal",
            variable=self.exec_mode, value="normal",
            command=self.update_buttons
        )
        rb_step = ttk.Radiobutton(
            frame_mode, text="Passo a Passo",
            variable=self.exec_mode, value="step",
            command=self.update_buttons
        )
        rb_normal.pack(anchor="w", padx=5, pady=5)
        rb_step.pack(anchor="w", padx=5, pady=5)

        # Botões – mais largos e mais para a direita
        self.btn_exec = ttk.Button(self.root, text="Executar", command=self.run_program)
        self.btn_exec.place(x=820, y=490, width=220, height=40)

        self.btn_step = ttk.Button(self.root, text="Próximo Passo", command=self.step_program)
        self.btn_step.place(x=820, y=540, width=220, height=40)

    # =========================
    # Habilitar/Desabilitar botões
    # =========================

    def update_buttons(self):
        mode = self.exec_mode.get()
        if mode == "normal":
            self.btn_exec.config(state="normal")
            self.btn_step.config(state="disabled")
        else:
            self.btn_exec.config(state="disabled")
            self.btn_step.config(state="normal")

    # =========================
    # Carregar programa
    # =========================

    def load_obj_file(self):
        filename = filedialog.askopenfilename(
            title="Selecione arquivo .obj",
            filetypes=[("Objeto MVD", "*.obj"), ("Todos", "*.*")]
        )
        if not filename:
            return

        try:
            with open(filename, "r", encoding="utf-8") as f:
                self.current_program_text = f.read()

            self.vm = MVDMachine(program_text=self.current_program_text, input_values=[])

            self.refresh_code_view()
            self.refresh_memory_view()
            self.refresh_output_view(clear=True)
            self.highlight_current_instruction()

            messagebox.showinfo("Sucesso", f"Programa carregado:\n{filename}")
        except Exception as e:
            messagebox.showerror("Erro", f"Falha ao carregar .obj:\n{e}")
            self.vm = None

    # =========================
    # JANELA DE ENTRADA PARA RD
    # =========================

    def ensure_input_for_rd(self) -> bool:
        if not self.vm:
            return False

        if not (0 <= self.vm.i < len(self.vm.P)):
            return False

        ins = self.vm.P[self.vm.i]

        if ins.op != "RD":
            return True

        if self.vm.input_values:
            return True

        # POPUP MAIOR PERSONALIZADO
        popup = tk.Toplevel(self.root)
        popup.title("Entrada para RD")
        popup.geometry("450x180")
        popup.resizable(False, False)

        lbl = ttk.Label(popup, text="Digite o valor inteiro para RD:", font=("Arial", 13))
        lbl.pack(pady=15)

        entry = ttk.Entry(popup, font=("Arial", 16), width=20)
        entry.pack(pady=5)
        entry.focus()

        result = {"value": None}

        def confirmar():
            text = entry.get().strip()
            try:
                v = int(text)
            except:
                messagebox.showerror("Erro", "Digite um número inteiro válido.")
                return
            result["value"] = v
            popup.destroy()

        def cancelar():
            result["value"] = None
            popup.destroy()

        btn_frame = tk.Frame(popup)
        btn_frame.pack(pady=15)

        ttk.Button(btn_frame, text="OK", width=12, command=confirmar).grid(row=0, column=0, padx=8)
        ttk.Button(btn_frame, text="Cancelar", width=12, command=cancelar).grid(row=0, column=1, padx=8)

        popup.grab_set()
        popup.wait_window()

        if result["value"] is None:
            return False

        self.vm.input_values.append(result["value"])
        return True

    # =========================
    # Execução NORMAL
    # =========================

    def run_program(self):
        if not self.vm:
            messagebox.showwarning("Atenção", "Nenhum programa carregado.")
            return

        if self.exec_mode.get() != "normal":
            messagebox.showinfo("Info", "Selecione 'Normal' para usar o botão Executar.")
            return

        self.refresh_output_view(clear=True)

        try:
            while self.vm.running and 0 <= self.vm.i < len(self.vm.P):
                if not self.ensure_input_for_rd():
                    break
                self.vm.step()

            self.refresh_memory_view()
            self.refresh_output_view()
            self.highlight_current_instruction()
        except Exception as e:
            messagebox.showerror("Erro em execução", str(e))

    # =========================
    # Execução PASSO A PASSO
    # =========================

    def step_program(self):
        if not self.vm:
            messagebox.showwarning("Atenção", "Nenhum programa carregado.")
            return

        if self.exec_mode.get() != "step":
            messagebox.showinfo("Info", "Selecione 'Passo a Passo' para usar este botão.")
            return

        if (not self.vm.running) or not (0 <= self.vm.i < len(self.vm.P)):
            messagebox.showinfo("Fim", "Programa já terminou (HLT alcançado).")
            return

        if not self.ensure_input_for_rd():
            return

        try:
            self.vm.step()
            self.refresh_memory_view()
            self.refresh_output_view()
            self.highlight_current_instruction()
        except Exception as e:
            messagebox.showerror("Erro em passo", str(e))

    # =========================
    # Atualização das views
    # =========================

    def refresh_code_view(self):
        for item in self.tree_code.get_children():
            self.tree_code.delete(item)

        if not self.vm:
            return

        for idx, ins in enumerate(self.vm.P):
            self.tree_code.insert(
                "",
                "end",
                iid=str(idx),
                values=(idx, ins.op, ins.a1 or "", ins.a2 or "", ins.label or "")
            )

    def highlight_current_instruction(self):
        for item in self.tree_code.selection():
            self.tree_code.selection_remove(item)

        if not self.vm:
            return

        idx = self.vm.i
        if 0 <= idx < len(self.vm.P):
            self.tree_code.selection_add(str(idx))
            self.tree_code.see(str(idx))

    def refresh_memory_view(self):
        for item in self.tree_mem.get_children():
            self.tree_mem.delete(item)

        if not self.vm:
            return

        topo = max(self.vm.s, 0)
        limite = max(topo + 1, 20)
        for addr in range(limite):
            self.tree_mem.insert("", "end", values=(addr, self.vm.M[addr]))

    def refresh_output_view(self, clear=False):
        if clear:
            self.txt_out.delete("1.0", tk.END)

        if not self.vm:
            return

        self.txt_out.delete("1.0", tk.END)
        if self.vm.outputs:
            self.txt_out.insert(tk.END, " ".join(str(v) for v in self.vm.outputs))


if __name__ == "__main__":
    root = tk.Tk()
    root.geometry("1100x650")
    app = MVDApp(root)
    root.mainloop()
