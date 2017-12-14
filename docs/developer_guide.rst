
Developer Guide
=================

Generating Documentation
------------------------

You will need the following Python packages.

.. code-block:: bash

    pip install recommonmark
    pip install sphinx
    pip install ghp-import
    pip install breathe

You will also need doxygen.

You can then type ``make html`` to create the documentation locally.

Submitting a Pull Request
-------------------------

From the develop branch of helics

- Make sure that you are in sync with the develop branch of the upstream remote.
- Make the required changes.
- Stage the changes (``git add``), commit the changes (``git commit``)
- Push the new commit and submit a PR on the github repository.

Opening an Issue
----------------

- Use the github issue tracker for submitting bugs or feature requests.
- Discuss on the gitter channel to chat with developers


