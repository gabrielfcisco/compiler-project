from dataclasses import dataclass
from typing import List, Dict, Optional, Union

@dataclass
class Instruction:
    label: Optional[str]
    op: str
    a1: Optional[Union[int, str]] = None
    a2: Optional[Union[int, str]] = None


# Máquina Virtual Didática (MVD)

class MVDMachine:
    def __init__(self, program_text: str, input_values: Optional[List[int]] = None, mem_size: int = 10000):
        # Memória de dados (pilha)
        self.M: List[int] = [0] * mem_size

        # Memória de programa (lista de Instruction)
        self.P: List[Instruction] = []

        # Tabela de rótulos: nome -> índice em P
        self.labels: Dict[str, int] = {}

        # Registradores
        self.i: int = 0      # program counter
        self.s: int = -1     # topo da pilha
        self.running: bool = True

        # Entrada/saída
        self.input_values: List[int] = input_values[:] if input_values else []
        self.outputs: List[int] = []

        # Montagem
        self._parse_program(program_text)
        self._resolve_labels()

    # Parsing do programa assembly

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

            # Separar ':' como token e remover vírgulas
            parts = line.replace(":", " : ").split()
            parts = [p.strip().rstrip(",") for p in parts if p.strip()]
            if not parts:
                continue

            label = None
            op = None
            a1 = None
            a2 = None

            # Detectar rótulo no início (L1: OP..., ou L1 OP...)
            if len(parts) >= 2 and parts[0].upper() not in opcodes:
                if parts[1] == ":":
                    label = parts[0]
                    parts = parts[2:]
                else:
                    label = parts[0]
                    parts = parts[1:]

            # Se sobrou só o label -> vira NULL
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

            # Registra rótulo apontando pra esse índice
            if instr.label is not None:
                if instr.label in self.labels:
                    raise ValueError(f"Rótulo duplicado: {instr.label}")
                self.labels[instr.label] = len(self.P)

            self.P.append(instr)

    def _resolve_labels(self):
        # Segunda passada: converte labels usados como operandos em endereços numéricos
        for idx, ins in enumerate(self.P):
            if ins.op in {"JMP", "JMPF", "CALL"} and isinstance(ins.a1, str):
                lab = ins.a1
                if lab not in self.labels:
                    raise ValueError(f"Rótulo não definido: {lab}")
                self.P[idx].a1 = self.labels[lab]

    # Execução

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

        # Próxima instrução padrão; desvios sobrescrevem
        next_i = self.i + 1

        #Instruções

        if op == "START":
            self.s = -1

        elif op == "HLT":
            self.running = False

        elif op == "NULL":
            pass

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

        elif op == "AND":
            self.M[self.s - 1] = 1 if (self.M[self.s - 1] == 1 and self.M[self.s] == 1) else 0
            self.s -= 1

        elif op == "OR":
            self.M[self.s - 1] = 1 if (self.M[self.s - 1] == 1 or self.M[self.s] == 1) else 0
            self.s -= 1

        elif op == "NEG":
            # NOT lógico: 1 -> 0, 0 -> 1
            self.M[self.s] = 1 - self.M[self.s]

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

        elif op == "JMP":
            next_i = to_int(a1)

        elif op == "JMPF":
            cond = self.M[self.s]
            self.s -= 1
            if cond == 0:
                next_i = to_int(a1)

        elif op == "RD":
            if not self.input_values:
                raise RuntimeError("Sem valores de entrada para RD")
            self.s += 1
            self.M[self.s] = self.input_values.pop(0)

        elif op == "PRN":
            val = self.M[self.s]
            self.s -= 1
            self.outputs.append(val)

        elif op == "ALLOC":
            # Formatos:
            #   ALLOC n      -> s := s + n
            #   ALLOC m,n    -> empilha cópia de M[m..m+n-1]
            if a2 is None:
                n = to_int(a1)
                self.s += n
            else:
                m = to_int(a1)
                n = to_int(a2)
                for k in range(n):
                    self.s += 1
                    self.M[self.s] = self.M[m + k]

        elif op == "DALLOC":
            # Formatos:
            #   DALLOC n     -> s := s - n
            #   DALLOC m,n   -> restaura M[m..m+n-1] a partir do topo
            if a2 is None:
                n = to_int(a1)
                self.s -= n
            else:
                m = to_int(a1)
                n = to_int(a2)
                for k in range(n - 1, -1, -1):
                    self.M[m + k] = self.M[self.s]
                    self.s -= 1

        elif op == "CALL":
            # Empilha endereço de retorno e salta
            self.s += 1
            self.M[self.s] = self.i + 1
            next_i = to_int(a1)

        elif op == "RETURN":
            # Usa topo como endereço de retorno
            ret = self.M[self.s]
            self.s -= 1
            next_i = ret

        elif op == "RETURNF":
            # Convenção:
            # topo da pilha: valor de retorno
            # abaixo: endereço de retorno
            # RESULT (v) permanece no topo para o chamador
            v = self.M[self.s]
            ret = self.M[self.s - 1]
            # remove v e ret
            self.s -= 2
            # empilha de volta só o resultado
            self.s += 1
            self.M[self.s] = v
            next_i = ret

        else:
            raise ValueError(f"Instrução não implementada: {op}")

        self.i = next_i
