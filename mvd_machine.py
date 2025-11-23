from dataclasses import dataclass
from typing import List, Dict, Optional, Union

@dataclass
class Instruction:
    label: Optional[str]
    op: str
    a1: Optional[Union[int, str]] = None
    a2: Optional[Union[int, str]] = None


class MVDMachine:
    def __init__(self, program_text: str,
                 input_values: Optional[List[int]] = None,
                 mem_size: int = 10000):
        # Memória de dados (variáveis + pilha)
        self.M: List[int] = [0] * mem_size

        # Memória de programa
        self.P: List[Instruction] = []

        # Tabela de rótulos
        self.labels: Dict[str, int] = {}

        # Registradores
        self.i: int = 0        # program counter
        self.s: int = -1       # topo da pilha (vai ser ajustado depois)
        self.running: bool = True

        # Entrada / saída
        self.input_values: List[int] = input_values[:] if input_values else []
        self.outputs: List[int] = []

        # Base da pilha (vamos calcular a partir dos endereços de variáveis)
        self.stack_base: int = 0

        # Montagem
        self._parse_program(program_text)
        self._compute_stack_base()
        self._resolve_labels()

        # Inicializa pilha acima da área de variáveis
        self.s = self.stack_base - 1

    # ---------------------------- PARSE ----------------------------

    def _parse_program(self, text: str):
        opcodes = {
            "LDC", "LDV", "STR",
            "ADD", "SUB", "MULT", "DIVI",
            "INV", "AND", "OR", "NEG",
            "CME", "CMA", "CEQ", "CDIF", "CMEQ", "CMAQ",
            "JMP", "JMPF", "NULL",
            "RD", "PRN",
            "START", "HLT",
            "ALLOC", "DALLOC",
            "CALL", "RETURN", "RETURNF"
        }

        for raw_line in text.splitlines():
            line = raw_line.strip()
            if not line:
                continue

            # Comentário com ';'
            if ";" in line:
                line = line.split(";", 1)[0].strip()
                if not line:
                    continue

            parts = line.replace(":", " : ").split()
            parts = [p.strip().rstrip(",") for p in parts if p.strip()]
            if not parts:
                continue

            label = None
            op = None
            a1 = None
            a2 = None

            # Rótulo no início (ex: "L1: ADD ..." ou "L1 ADD ...")
            if len(parts) >= 2 and parts[0].upper() not in opcodes:
                if parts[1] == ":":
                    label = parts[0]
                    parts = parts[2:]
                else:
                    label = parts[0]
                    parts = parts[1:]

            if not parts:
                op = "NULL"
            else:
                op = parts[0].upper()
                if op not in opcodes:
                    raise ValueError(f"Opcode desconhecido: {op} na linha: {raw_line}")

                if len(parts) >= 2:
                    a1 = parts[1]
                if len(parts) >= 3:
                    a2 = parts[2]

            instr = Instruction(label=label, op=op, a1=a1, a2=a2)

            if instr.label is not None:
                if instr.label in self.labels:
                    raise ValueError(f"Rótulo duplicado: {instr.label}")
                self.labels[instr.label] = len(self.P)

            self.P.append(instr)

    def _compute_stack_base(self):
        """
        Define uma base da pilha *acima* do maior endereço de variável usado.
        Considera como endereços de dados apenas os operandos de:
        LDV, STR, ALLOC, DALLOC.
        """
        max_addr = -1

        for ins in self.P:
            if ins.op not in {"LDV", "STR", "ALLOC", "DALLOC"}:
                continue

            for arg in (ins.a1, ins.a2):
                if arg is None:
                    continue
                # Só inteiros (ignora rótulos)
                if isinstance(arg, str) and arg.lstrip("-").isdigit():
                    val = int(arg)
                    if val > max_addr:
                        max_addr = val

        # Se não achou nada, deixa pilha começar em 0
        if max_addr < 0:
            self.stack_base = 0
        else:
            # margem de segurança
            self.stack_base = max_addr + 5

    def _resolve_labels(self):
        # Converte rótulos usados em JMP/JMPF/CALL em índices numéricos
        for idx, ins in enumerate(self.P):
            if ins.op in {"JMP", "JMPF", "CALL"} and isinstance(ins.a1, str):
                lab = ins.a1
                if lab not in self.labels:
                    raise ValueError(f"Rótulo não definido: {lab}")
                self.P[idx].a1 = self.labels[lab]

    # ---------------------------- EXECUÇÃO ----------------------------

    def run(self, max_steps: int = 100000) -> List[int]:
        steps = 0
        while self.running and 0 <= self.i < len(self.P):
            if steps >= max_steps:
                raise RuntimeError("Limite de passos excedido (possível loop infinito).")
            self.step()
            steps += 1
        return self.outputs

    def step(self):
        ins = self.P[self.i]
        op = ins.op
        a1 = ins.a1
        a2 = ins.a2

        def to_int(x):
            return int(x) if x is not None else 0

        next_i = self.i + 1

        # ---------------- START / HLT / NULL ----------------
        if op == "START":
            # agora a pilha sempre começa acima da área de variáveis
            self.s = self.stack_base - 1

        elif op == "HLT":
            self.running = False

        elif op == "NULL":
            pass

        # ---------------- CARREGAR / ARMAZENAR ----------------
        elif op == "LDC":
            self.s += 1
            self.M[self.s] = to_int(a1)

        elif op == "LDV":
            addr = to_int(a1)
            self.s += 1
            self.M[self.s] = self.M[addr]

        elif op == "STR":
            addr = to_int(a1)
            self.M[addr] = self.M[self.s]
            self.s -= 1

        # ---------------- ARITMÉTICAS ----------------
        elif op == "ADD":
            self.M[self.s - 1] = self.M[self.s - 1] + self.M[self.s]
            self.s -= 1

        elif op == "SUB":
            self.M[self.s - 1] = self.M[self.s - 1] - self.M[self.s]
            self.s -= 1

        elif op == "MULT":
            self.M[self.s - 1] = self.M[self.s - 1] * self.M[self.s]
            self.s -= 1

        elif op == "DIVI":
            if self.M[self.s] == 0:
                raise ZeroDivisionError("DIVI por zero")
            self.M[self.s - 1] = self.M[self.s - 1] // self.M[self.s]
            self.s -= 1

        elif op == "INV":
            self.M[self.s] = -self.M[self.s]

        # ---------------- LÓGICAS ----------------
        elif op == "AND":
            self.M[self.s - 1] = 1 if (self.M[self.s - 1] == 1 and self.M[self.s] == 1) else 0
            self.s -= 1

        elif op == "OR":
            self.M[self.s - 1] = 1 if (self.M[self.s - 1] == 1 or self.M[self.s] == 1) else 0
            self.s -= 1

        elif op == "NEG":
            self.M[self.s] = 1 - self.M[self.s]

        # ---------------- COMPARAÇÕES ----------------
        elif op == "CME":
            self.M[self.s - 1] = 1 if self.M[self.s - 1] < self.M[self.s] else 0
            self.s -= 1

        elif op == "CMA":
            self.M[self.s - 1] = 1 if self.M[self.s - 1] > self.M[self.s] else 0
            self.s -= 1

        elif op == "CEQ":
            self.M[self.s - 1] = 1 if self.M[self.s - 1] == self.M[self.s] else 0
            self.s -= 1

        elif op == "CDIF":
            self.M[self.s - 1] = 1 if self.M[self.s - 1] != self.M[self.s] else 0
            self.s -= 1

        elif op == "CMEQ":
            self.M[self.s - 1] = 1 if self.M[self.s - 1] <= self.M[self.s] else 0
            self.s -= 1

        elif op == "CMAQ":
            self.M[self.s - 1] = 1 if self.M[self.s - 1] >= self.M[self.s] else 0
            self.s -= 1

        # ---------------- DESVIOS ----------------
        elif op == "JMP":
            next_i = to_int(a1)

        elif op == "JMPF":
            cond = self.M[self.s]
            self.s -= 1
            if cond == 0:
                next_i = to_int(a1)

        # ---------------- I/O ----------------
        elif op == "RD":
            if not self.input_values:
                raise RuntimeError("Sem valores de entrada para RD")
            self.s += 1
            self.M[self.s] = self.input_values.pop(0)

        elif op == "PRN":
            val = self.M[self.s]
            self.s -= 1
            self.outputs.append(val)

        # ---------------- ALLOC / DALLOC ----------------
        elif op == "ALLOC":
            if a2 is None:
                raise ValueError("Erro: ALLOC requer dois parâmetros (m n).")
            m = to_int(a1)
            n = to_int(a2)
            for k in range(n):
                self.s += 1
                self.M[self.s] = self.M[m + k]

        elif op == "DALLOC":
            if a2 is None:
                raise ValueError("Erro: DALLOC requer dois parâmetros (m n).")
            m = to_int(a1)
            n = to_int(a2)
            for k in range(n - 1, -1, -1):
                self.M[m + k] = self.M[self.s]
                self.s -= 1

        # ---------------- CALL / RETURN ----------------
        elif op == "CALL":
            self.s += 1
            self.M[self.s] = self.i + 1
            next_i = to_int(a1)

        elif op == "RETURN":
            ret = self.M[self.s]
            self.s -= 1
            next_i = ret

        elif op == "RETURNF":
            v = self.M[self.s]
            ret = self.M[self.s - 1]
            self.s -= 2
            self.s += 1
            self.M[self.s] = v
            next_i = ret

        else:
            raise ValueError(f"Instrução não implementada: {op}")

        self.i = next_i
