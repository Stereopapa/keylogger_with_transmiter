

def test_interpreter_backspace():
    from app.services.keystrokes_interpreter import KeystrokesInterpreter

    inter: KeystrokesInterpreter = KeystrokesInterpreter()
    inter.set_conf(backspace_deletes=True)
    # assert("[dfs]" == inter.interprete("[dfs]"))
    # assert (""  == inter.interprete("[BACKSPACE]"))
    # assert ("" == inter.interprete("1[BACKSPACE]"))
    # assert ("1" == inter.interprete("12[BACKSPACE]"))
    # assert ("12" == inter.interprete("12[BACKSPACE]2"))
    # assert ("1" == inter.interprete("12[BACKSPACE]2[BACKSPACE]"))
    # assert ("" == inter.interprete("12[BACKSPACE]2[BACKSPACE][BACKSPACE]"))
    assert ("" == inter.interprete("[BACKSPACE]12[BACKSPACE]2[BACKSPACE][BACKSPACE]"))
    assert ("[DELETE]" == inter.interprete("[DELETE]"))
    assert ("[DELETE]" == inter.interprete("[DELETE][BACKSPACE]"))