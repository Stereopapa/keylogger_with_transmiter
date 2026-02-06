from typing import List, TypedDict, Unpack





class KeystrokesInterpreter:
    _special_keys: List[str] = [
        "[BACKSPACE]", "[ENTER]", "[TAB]", "[ESC]", "[CAPSLOCK]",
        "[DELETE]", "[INSERT]", "[HOME]", "[END]", "[PAGEUP]",
        "[PAGEDOWN]", "[PRINTSCREEN]", "[SCROLLLOCK]", "[PAUSE]",
        "[LEFT]", "[RIGHT]", "[UP]", "[DOWN]", "[F1]", "[F2]", "[F3]",
        "[F4]", "[F5]", "[F6]", "[F7]", "[F8]", "[F9]", "[F10]", "[F11]",
        "[F12]", "[NUM0]", "[NUM1]", "[NUM2]", "[NUM3]", "[NUM4]",
        "[NUM5]", "[NUM6]", "[NUM7]", "[NUM8]", "[NUM9]", "[NUM*]",
        "[NUM+]", "[NUM_SEPARATOR]", "[NUM-]", "[NUM.]", "[NUM/]", "[NUMLOCK]"]
    _backspace_deletes: bool = True
    _max_spec_keys = 15

    class InterpreterConfig(TypedDict, total=False):
        backspace_deletes: bool

    def set_conf(self, **kwargs: Unpack[InterpreterConfig]):
        if conf := kwargs["backspace_deletes"]: self._backspace_deletes = conf

    def __init__(self):
        pass


    def interprete(self, keystrokes: str) -> str:
        if self._backspace_deletes: keystrokes = self._delete_before_backspace(keystrokes)

        return keystrokes

    def _delete_before_backspace(self, keystrokes: str) -> str:
        to_find = "[BACKSPACE]"

        while (b_start := keystrokes.find(to_find)) >= 0:
            keystrokes = keystrokes[:b_start] + keystrokes[b_start + len(to_find):]
            if b_start == 0: continue
            elif b_start - 1 > 0 and keystrokes[b_start-1] == "]":
                i_max = b_start - 1
                spec_key = ""
                while keystrokes !=  '[' or i_max > b_start - 1 - self._max_spec_keys:
                    spec_key += keystrokes[i_max]
                    i_max -= 1

                spec_key = spec_key[::-1]
                if self._special_keys.index(spec_key): continue

            keystrokes = keystrokes[:b_start-1] + keystrokes[b_start:]
        return keystrokes