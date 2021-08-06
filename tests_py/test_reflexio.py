from .app_utils_test import *


def test_reflexio():
    my_struct1 = MyStruct()
    my_struct2 = MyStruct()

    assert(my_struct1 == my_struct2)
    assert(my_struct1.get_serial_size() == my_struct2.get_serial_size())

    my_struct1.var6[-1] = 2    
    my_struct1.var7 = VectorFloat([1,2,3])
    print(my_struct1)
    assert(my_struct1 != my_struct2)
    assert(my_struct1.get_serial_size() == my_struct2.get_serial_size() + 3 * 4)
