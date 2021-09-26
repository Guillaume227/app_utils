from . import app_utils_test
from .app_utils_test import *


def test_reflexio():
    my_struct1 = MyStruct()
    my_struct2 = MyStruct()

    assert(my_struct1.var3 == MyEnum.EnumVal2)
    assert(my_struct1.var4 == MyOtherEnum.EnumVal2)
    print(my_struct1.var4)
    assert(my_struct1 == my_struct2)
    assert(my_struct1.get_serial_size() == my_struct2.get_serial_size())

    my_struct1.var6[-1] = 2
    assert(my_struct1.var6[-1] == 2)

    if hasattr(app_utils_test, 'CONSTEXPR_STRING_AND_VECTOR'):
        my_struct1.var7 = "hello hello"
        my_struct1.var8 = VectorFloat([1, 2, 3])
        assert(my_struct1.get_serial_size() == my_struct2.get_serial_size() 
               + 3 * 4 # Vector of floats
               + len(my_struct1.var7) - len(my_struct2.var7)) # difference in string size

    print(my_struct1)
    assert(my_struct1 != my_struct2)
