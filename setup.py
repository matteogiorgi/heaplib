from setuptools import Extension, setup


HPQLIB_SOURCES = [
    "src/priority_queue.c",
    "src/heaps/binary_heap.c",
    "src/heaps/fibonacci_heap.c",
    "src/heaps/kaplan_heap.c",
]

HPQLIB_HEADERS = [
    "include",
    "src",
]

HPQLIB_EXTENSION = Extension(
    "hpqlib",
    sources=["python/hpqlibmodule.c", *HPQLIB_SOURCES],
    include_dirs=HPQLIB_HEADERS,
    extra_compile_args=["-std=c99", "-Wall", "-Wextra", "-pedantic"],
)


setup(
    packages=[],
    py_modules=[],
    ext_modules=[HPQLIB_EXTENSION],
)
