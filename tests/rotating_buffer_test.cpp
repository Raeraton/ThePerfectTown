
#include <cassert>
#include <iostream>
#include <string>

#include <rotating_buffer.hpp>

using util::Rotating_buffer;

void test_basic_insertion()
{
    Rotating_buffer<int> buffer(5);

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);

    assert(buffer[0] == 10);
    assert(buffer[1] == 20);
    assert(buffer[2] == 30);

    assert(buffer.first() == 10);
    assert(buffer.last() == 30);
}

void test_remove_last()
{
    Rotating_buffer<int> buffer(5);

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);

    buffer.remove_last();

    assert(buffer[0] == 10);
    assert(buffer[1] == 20);
    assert(buffer.last() == 20);

    buffer.remove_last();

    assert(buffer.last() == 10);
}

void test_remove_first()
{
    Rotating_buffer<int> buffer(5);

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);

    buffer.remove_first();

    assert(buffer[0] == 20);
    assert(buffer[1] == 30);

    assert(buffer.first() == 20);
    assert(buffer.last() == 30);
}

void test_buffer_overflow()
{
    Rotating_buffer<int> buffer(3);

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);

    // Buffer is full. Oldest element should be overwritten.
    buffer.add(40);

    assert(buffer[0] == 20);
    assert(buffer[1] == 30);
    assert(buffer[2] == 40);

    buffer.add(50);

    assert(buffer[0] == 30);
    assert(buffer[1] == 40);
    assert(buffer[2] == 50);
}

void test_wraparound()
{
    Rotating_buffer<int> buffer(3);

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);

    buffer.remove_first();

    // Physical buffer: [10, 20, 30]
    // Logical buffer:  [20, 30]

    buffer.add(40);

    // Physical buffer should wrap around.
    assert(buffer[0] == 20);
    assert(buffer[1] == 30);
    assert(buffer[2] == 40);

    buffer.remove_first();
    buffer.add(50);

    assert(buffer[0] == 30);
    assert(buffer[1] == 40);
    assert(buffer[2] == 50);
}

void test_remove_and_reuse()
{
    Rotating_buffer<int> buffer(3);

    buffer.add(10);
    buffer.add(20);
    buffer.add(30);

    buffer.remove_first();
    buffer.remove_last();

    buffer.add(40);
    buffer.add(50);

    assert(buffer[0] == 20);
    assert(buffer[1] == 40);
    assert(buffer[2] == 50);
}

void test_capacity_one()
{
    Rotating_buffer<int> buffer(1);

    buffer.add(10);
    assert(buffer[0] == 10);

    buffer.add(20);
    assert(buffer[0] == 20);

    buffer.add(30);
    assert(buffer[0] == 30);

    buffer.remove_first();
    buffer.add(40);

    assert(buffer[0] == 40);
}

void test_string_type()
{
    Rotating_buffer<std::string> buffer(3);

    buffer.add("Hello");
    buffer.add("World");

    assert(buffer[0] == "Hello");
    assert(buffer[1] == "World");

    assert(buffer.first() == "Hello");
    assert(buffer.last() == "World");
}

void test_const_access()
{
    Rotating_buffer<int> buffer(3);

    buffer.add(10);
    buffer.add(20);

    const Rotating_buffer<int>& const_buffer = buffer;

    assert(const_buffer[0] == 10);
    assert(const_buffer[1] == 20);

    assert(const_buffer.first() == 10);
    assert(const_buffer.last() == 20);
}

int main()
{
    test_basic_insertion();
    test_remove_last();
    test_remove_first();
    test_buffer_overflow();
    test_wraparound();
    test_remove_and_reuse();
    test_capacity_one();
    test_string_type();
    test_const_access();

    std::cout << "All tests passed!\n";
}
