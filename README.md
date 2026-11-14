# CS 118 Fall 26 Project 2

This repository contains the baseline skeleton code for Project 2. We will add in our solution to Project 1 on Nov 18, 2026 (after the late deadline of Project 1). Feel free to use your own Project 1 to get started. 

`security.c` and `security.h` are wrappers around OpenSSL 3's `libcrypto` library. Any of the complicated cryptography mechanics have already been implemented for you. Feel free to read their descriptions in `security.h`. When submitting, make sure to add these files to your Makefile and your ZIP file.

In the `keys` directory, you'll see the files mentioned by the spec. Place these in the current working directory of wherever you're testing. For example, if you run `./client` and your PWD is `/Users/mrElnekave`, make sure `ca_public_key.bin` exists in `/Users/mrElnekave`. Note that the autograder automatically generates these files--do not rely on the exact contents of them. Read the [spec](https://docs.google.com/document/d/1wXKU--xdpIZhGmexrLpmSNx1uVuNIHE-obvViN_22Ek/edit?usp=sharing) for more info.
