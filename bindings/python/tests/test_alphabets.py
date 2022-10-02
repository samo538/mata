__author__ = 'Tomas Fiedor'

import mata
import pytest


def test_on_the_fly_alphabet_with_character_symbols():
    """
    Tests on the fly alphabet with character symbols.

    OnTheFlyAlphabet translates the symbols into values on-the-fly, based on a given counter.
    """
    alphabet = mata.OnTheFlyAlphabet()
    assert alphabet.translate_symbol("'a'") == 0
    assert alphabet.translate_symbol("'b'") == 1
    assert alphabet.translate_symbol("b") == 2
    assert alphabet.translate_symbol("1") == 3
    assert alphabet.translate_symbol("10") == 4
    assert alphabet.translate_symbol("ahoj") == 5
    assert alphabet.translate_symbol('"a"') == 6
    assert alphabet.translate_symbol('"0"') == 7


def test_on_the_fly_alphabet_with_enumeration_of_symbols():
    """
    Tests on the fly alphabet.

    OnTheFlyAlphabet translates the symbols into values on-the-fly, based on a given counter.
    """
    alphabet = mata.OnTheFlyAlphabet.from_symbol_map({'a': 0, 'b': 1, 'c': 2})
    assert alphabet.translate_symbol('a') == 0
    assert alphabet.translate_symbol('b') == 1
    assert alphabet.translate_symbol('c') == 2
    # TODO: Decide on a unified format of specifying that the alphabet cannot be modified (new symbols cannot be added)
    #  and propagate the information to the Python binding as well.
    #with pytest.raises(RuntimeError):
    #    assert alphabet.translate_symbol('d') == 2


def test_on_the_fly_alphabet():
    """Tests on the fly alphabet

    OnTheFlyAlphabet translates the symbols into values on-the-fly,
    based on a given counter.
    """
    alphabet = mata.OnTheFlyAlphabet()
    assert alphabet.translate_symbol('a') == 0
    assert alphabet.translate_symbol('a') == 0
    assert alphabet.translate_symbol('b') == 1
    assert alphabet.translate_symbol('a') == 0
    assert alphabet.translate_symbol('c') == 2

    alphabet = mata.OnTheFlyAlphabet(3)
    assert alphabet.translate_symbol('a') == 3
    assert alphabet.translate_symbol('b') == 4
    assert alphabet.translate_symbol('c') == 5
    assert alphabet.translate_symbol('a') == 3
