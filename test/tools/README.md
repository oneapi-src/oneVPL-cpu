# Test Tools

This folder contains tools used by tests.

## Python Tools

Tools in the Python folder are designed to be called either form the command
line, or as libraries. The entire test/tools/python folder can be added to
PYTHONPATH to expose these tools in the environment.

the vars.* scripts in test/env will add test/tools/python to PYTHONPATH
as part of their environment setup.

ex:

```bash
py -3 -m test/tools/python/get_content file://foo.bar

#or

python3 -m test/tools/python/get_content file://foo.bar

```

or 

```python
import get_content
get_content("file://foo.bar")
```

