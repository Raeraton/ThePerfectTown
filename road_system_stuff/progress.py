import sys


class ProgressBar:
    def __init__(self, label: str, total: int | None = None, width: int = 32):
        self.label = label
        self.total = total
        self.width = width
        self.current = 0

    def start(self) -> None:
        if self.total is None:
            sys.stdout.write(f"{self.label}: ...")
        else:
            self.update(0)
        sys.stdout.flush()

    def update(self, current: int, detail: str | None = None) -> None:
        self.current = current
        if self.total is None:
            return

        percent = 100 if self.total <= 0 else min(100, int(current * 100 / self.total))
        filled = int(self.width * percent / 100)
        bar = "█" * filled + "░" * (self.width - filled)
        suffix = f" {detail}" if detail else ""
        sys.stdout.write(f"\r{self.label}: [{bar}] {percent:3}% ({current}/{self.total}){suffix}")
        sys.stdout.flush()

    def advance(self, amount: int = 1, detail: str | None = None) -> None:
        self.update(self.current + amount, detail)

    def finish(self, detail: str | None = None) -> None:
        if self.total is None:
            sys.stdout.write(" done\n")
        else:
            self.update(self.total, detail)
            sys.stdout.write("\n")
        sys.stdout.flush()
