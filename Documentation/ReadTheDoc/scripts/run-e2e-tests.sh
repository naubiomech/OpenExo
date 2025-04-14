#!/bin/bash

extraArgs=$@

pytest -s ./e2e/test_homepage.py $extraArgs
