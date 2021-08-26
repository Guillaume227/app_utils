from . import app_utils_test
from .app_utils_test import *


def test_reflexio():

    assert(str(MyEnum.EnumVal1) == 'MyEnum.EnumVal1')
    assert(int(MyEnum.EnumVal1) == 1)

    assert(str(MyEnum.EnumVal2) == 'MyEnum.EnumVal2')
    assert(int(MyEnum.EnumVal2) == 2)

    assert(str(MyEnum.EnumVal3) == 'MyEnum.EnumVal3')
    assert(int(MyEnum.EnumVal3) == 3)

    assert(str(MyEnum.EnumVal4) == 'MyEnum.EnumVal4')
    assert(int(MyEnum.EnumVal4) == 5)
    