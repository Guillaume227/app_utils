from app_utils_test import *
import app_utils_test


def test_reflexio():
    my_struct1 = MyStruct()
    my_struct2 = MyStruct()

    assert(my_struct1.var3 == MyEnum.EnumVal2)
    assert(my_struct1.var4 == MyOtherEnum.EnumVal2)
    print(my_struct1.var4)
    assert(my_struct1 == my_struct2)
    assert(my_struct1.get_serial_size() == my_struct2.get_serial_size())
   
    # pointer semantics check for array
    my_struct1.var6[-1] = 2
    assert(my_struct1.var6[-1] == 2), "in-place update did not work"

    # value semantics check for arithmetic types
    previous_value = my_struct1.var1
    new_value = previous_value * 2
    my_struct1.var1 = new_value
    assert(previous_value != new_value), f"expected value semantics but {previous_value} == {new_value}"
    assert(previous_value != my_struct1.var1), f"expected value semantics but {previous_value} == {my_struct1.var1}"

    # value semantics check for wrapped float type
    previous_value = my_struct1.var7
    new_value = previous_value / 2
    my_struct1.var7 = new_value
    assert(previous_value != new_value), f"expected value semantics but {previous_value} == {new_value}"
    assert(previous_value != my_struct1.var7), f"expected value semantics but {previous_value} == {my_struct1.var7}"

    # pointer semantics check for wrapped float type with different return value policy
    previous_value = my_struct1.var8
    new_value = previous_value / 2
    my_struct1.var8 = new_value
    assert(previous_value == new_value), f"expected reference semantics but {previous_value} != {new_value}"
    assert(previous_value == my_struct1.var8), f"expected reference semantics but {previous_value} != {my_struct1.var8}"

    if hasattr(app_utils_test, 'CONSTEXPR_STRING_AND_VECTOR'):
        my_struct1.var_string = "hello hello"
        my_struct1.var_vect = VectorFloat([1, 2, 3])
        assert(my_struct1.get_serial_size() == my_struct2.get_serial_size() 
               + 3 * 4 # Vector of floats
               + len(my_struct1.var_string) - len(my_struct2.var_string)) # difference in string size

    print(my_struct1)
    assert(my_struct1 != my_struct2)


def test_reflexio_nested():
    nested_struct1 = NestedStruct()
    nested_struct2 = NestedStruct()

    assert(nested_struct1 == nested_struct2)

    var1 = nested_struct1.var1
    var1.var1 = 33

    assert(nested_struct1 != nested_struct2)

    assert(list(nested_struct1.differing_members(nested_struct2)) == ['var1'])
    assert(list(nested_struct1.var1.differing_members(nested_struct2.var1)) == ['var1'])    

    nested_struct1.var1 = nested_struct2.var1
    assert(nested_struct1 == nested_struct2)

    nested_struct2.var2.var1 = 555

    assert(nested_struct1 != nested_struct2)
    assert(list(nested_struct1.var2.differing_members(nested_struct2.var2)) == ['var1'])


def test_reflexio_as_dict():

    my_struct1 = MyStruct()
    dico = my_struct1.as_dict()
    for item in dico:
        print(item, dico[item])

    var6_val = 222
    dico['var6'][2] = var6_val
    assert(my_struct1.var6[2] == var6_val), 'vector should have pointer semantics'

    if hasattr(app_utils_test, 'CONSTEXPR_STRING_AND_VECTOR'):
        # strings not supported on linux yet due to imperfect c++20 gcc compliance
        dico['var_string'] = dico['var_string'] + '_suffix'
        assert(my_struct1.var_string != 'var_string_val_suffix'), "python strings have value semantics"

    new_var2_val = 111.
    assert(getattr(my_struct1, 'var2') != new_var2_val)
    assert(my_struct1['var2'] != new_var2_val)

    my_struct1['var2'] = new_var2_val
    assert(getattr(my_struct1, 'var2') == new_var2_val)
    assert(my_struct1['var2'] == new_var2_val)
    assert(my_struct1.var2 == new_var2_val)

    print('labels:')
    for label in my_struct1:
        print(label)


def test_numpy():
    from app_utils_test import SimpleStruct, MyEnum, get_simple_struct_array
    import numpy as np
    assert(SimpleStruct.dtype == np.dtype([('var1', '<i4'), ('var3', '<i4')]))
    a = np.array([(12, MyEnum.EnumVal1), (3, MyEnum.EnumVal2)], dtype=SimpleStruct.dtype)
    assert((a['var1'] == [12, 3]).all())
    assert((a['var3'] == [int(MyEnum.EnumVal1), int(MyEnum.EnumVal2)]).all())

    arr = get_simple_struct_array(5)
    print(arr)
    assert((arr['var1'] == [0, 1, 2, 3, 4]).all())
    assert(arr.dtype == SimpleStruct.dtype)