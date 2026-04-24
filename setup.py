from pathlib import Path

from setuptools import Extension, setup


ROOT = Path(__file__).parent
README = ROOT / "README.md"

PQLIB_EXTENSION = Extension(
    "pqlib",
    sources=[
        "python/pqlibmodule.c",
        "src/priority_queue.c",
        "src/heaps/binary_heap.c",
        "src/skiplists/randomized_skiplist.c",
    ],
    include_dirs=["include", "src"],
    extra_compile_args=["-std=c99", "-Wall", "-Wextra", "-pedantic"],
)


setup(
    name="pqlib",
    version="0.1.0",
    description="C99 priority-queue library with Python bindings",
    long_description=README.read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    python_requires=">=3.8",
    ext_modules=[PQLIB_EXTENSION],
)
