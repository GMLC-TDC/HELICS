# Encrypted Communication

> **Warning**
> This guide is only meant to show how to use the encryption features in HELICS, and is not a substitute for
> getting help from a security expert to make sure the way you use the encryption features (generating keys, signing
> certificates, etc) is actually secure. In particular, any self-signed certificates or private key files you find
> online or in the HELICS repository **are for testing purposes only** and should not be used if secure communication
> is required.

Some applications may exchange data between federates that requires encryption in order to ensure its confidentiality.
This could be of particular concern when the federates are communicating over the public internet between different
institutions, and restrictions on a data set or simulator prohibit the co-simulation from being executed within a private
network at a single institution.

The HELICS TCP and TCPSS cores support encryption using the OpenSSL library. By default peer verification is used
which requires both the HELICS broker and federates using encrypted communication to be set up with their own certificate
and private key, which also allows the encrypted communication feature to act as a form of authentication between a broker
and the federates connecting to it.

This guide will provide information on building a copy of HELICS with encryption support, and show how to run a simulation
with encrypted communication. It will not go into best practices for generating and distributing certificates or private
keys. It is your responsibility to ensure that private keys and certificates are managed in a way that does not compromise
the security of the encrypted communication. A good starting point for learning more about this topic is reading up on
Public Key Infrastructure (PKI) and SSL/TLS fundamentals.

## Building HELICS with Encryption Support

To build HELICS with encryption support, you must have a copy of OpenSSL installed on your system. HELICS has been tested
using OpenSSL 1.1; OpenSSL 3 was recently released and includes breaking changes to the library API, though based on initial
testing it seems to work with HELICS.

With most Linux package managers you will want to install the `libssl-dev` and `openssl` packages; the former provides
the shared libraries and header files needed (including `libcrypto` and `libssl`), and the latter installs the `openssl`
command which provides tools for managing SSL certificates and private keys.

On macOS the system copy of libssl is likely pretty old, so Homebrew should be used to install the `openssl@1.1` package, which
includes a much more recent copy of libssl that includes fixes for a number of vulnerabilities.

For Windows users, a precompiled OpenSSL installer can be downloaded from [https://slproweb.com/products/Win32OpenSSL.html](https://slproweb.com/products/Win32OpenSSL.html).
The full version of the installer is needed (**NOT THE LIGHT VERSION**) which includes the required OpenSSL header files, and
the Win64 version is recommended for almost all users.

After that, continue as usual for [building HELICS from source](../installation/build_from_source.md) but at the CMake configure
step turn on the `HELICS_ENABLE_ENCRYPTION` option. In most cases CMake will find the installed copy of OpenSSL without errors,
but if not CMake will give an error message saying what options needs to be set to manually specify where OpenSSL was installed.

## Enabling Encryption

After successfully building and installing a copy of HELICS with encryption support, encryption can be enabled for the HELICS
broker and federates either via command line arguments, environment variables, federate configuration files, or modifying the
code for custom federates. Depending on your set up for launching a co-simulation, a combination of these methods may be used.

### Command-line

Encryption can be enabled by providing the `--encrypted` command-line flag to the `helics_broker` binary, or a HELICS federate
that accepts command line arguments (such as one of the `helics_app` subcommands). When encryption is disabled (the default
if no `--encrypted` flag or other mechanism is used to enable it), any other encryption related settings or arguments will be
ignored.

Along with enabling encryption, an encryption config file must be provided with information on private key and certificate files.
The `--encryption_config` command line argument can be used to specify the name and location of the encryption configuration
file that should be used.

As an example, here is what the command line arguments look when starting a HELICS broker and HELICS echo app (assuming the
encryption config file is named `openssl.json` and located in the current working directory):

```bash
helics_broker --encrypted --encryption_config=openssl.json
helics_app echo --encrypted --encryption_config=openssl.json
```

### Environment Variables

As an alternative to providing command-line arguments for each broker and federate launched, the `HELICS_ENCRYPTION` and
`HELICS_ENCRYPTION_CONFIG` environment variables can be set as alternatives to either or both of the `--encrypted` and
`--encryption_config` arguments. This can make it easier to change the setting for many federates all at the same time,
and require less typing when manually launching a co-simulation.

To set these environment variables, the following commands can be used on most Linux/macOS shells (on Windows the command will
depend on if you are using Command Prompt, Powershell, or Windows Subsystem for Linux):

```bash
export HELICS_ENCRYPTION=true
export HELICS_ENCRYPTION_CONFIG="$HOME/example-directory/openssl.json"
```

After that, a `helics_broker` or federate can be run as usual without needing to explicitly provide `--encrypted` or
`--encryption_config` arguments on the command line. As an additional note, using an absolute path to the encryption config
file will help reduce the chance of errors that might happen if different federates can run from different directories.

### Federate Configuration Files

The `encrypted` and `encryption_config` options can also be set from a federate configuration file, such as this:

```json
{
  "encrypted": true,
  "encryption_config": "/home/username/example-directory/openssl.json"
}
```

### Federate Code

Finally, it is also possible to set these options in the code of a federate (example shown in C++, but similar
mechanisms can also be used from other language interfaces).

Using a `FederateInfo` object named `fi`, this might look like:

```cpp
fi.encrypted = true;
fi.encryptedConfig = "/home/username/example-directory/openssl.json";
```

Or, the federate info args string can provide the settings in the form of a string matching the command-line arguments
described earlier:

```cpp
const std::string fedArgs =
    "--core_type=tcp --encrypted --encryption_config=" + std::string(TEST_BIN_DIR) +
    "encryption_config/openssl.json";
helics::FederateInfo fi_enc(fedArgs);
helics::ValueFederate fed1("fed1", fi_enc);
```

To check if the HELICS library being linked to has encryption support, the `helics-config.h` header can be imported,
which will define `HELICS_ENABLE_ENCRYPTION` if encryption is supported:

```cpp
#include "helics/helics-config.h"

#ifdef HELICS_ENABLE_ENCRYPTION
// <code that requires HELICS compiled with encryption support>
#endif
```

If CMake is being used, the following snippet (taken from the main HELICS repository `tests/helics/CMakeLists.txt`
file around lines 37-46) shows how CMake's `configure_file` function can be used to automatically fill in the
location of certificate and private key files (WARNING: do NOT put private key files in a public repository; ensuring
you are using the encryption features securely is your own responsibility):

```cmake
if(HELICS_ENABLE_ENCRYPTION)
    configure_file(
        "test_files/encryption_config/openssl.json.in" "test_files/encryption_config/openssl.json"
    )
    configure_file(
        "test_files/encryption_config/multiBroker_encrypted_bridge.json.in"
        "test_files/encryption_config/multiBroker_encrypted_bridge.json"
    )
endif()
```

And the corresponding input `openssl.json.in` file looks like:

```json
{
  "encrypted": true,
  "verify_file": "${CMAKE_CURRENT_SOURCE_DIR}/test_files/encryption_config/openssl_certs/ca.pem",
  "certificate_chain_file": "${CMAKE_CURRENT_SOURCE_DIR}/test_files/encryption_config/openssl_certs/server.pem",
  "private_key_file": "${CMAKE_CURRENT_SOURCE_DIR}/test_files/encryption_config/openssl_certs/server.pem",
  "tmp_dh_file": "${CMAKE_CURRENT_SOURCE_DIR}/test_files/encryption_config/openssl_certs/dh4096.pem",
  "password": "test"
}
```

Another big point of CAUTION: do NOT use the example/test SSL certificates or private key files provided in the HELICS
repository! These files are **for testing only** with data that does not require protection. By using them with a real
use case, you might as well not be using any encryption at all.

## Configuring Encryption Settings for a Co-simulation

Besides enabling encryption and providing an encryption configuration file using one of the previously described
methods, there are a few settings that the encryption configuration file should provide.

The general outline of what the file looks is:

```text
{
    "encrypted": <true|false; this setting is optional and sort of an alternative to the --encrypted flag so can be omitted>,
    "verify_file": "<path to certificate authority (CA) file to verify connecting federates>",
    "certificate_chain_file": "<path to certificate file to send to other federates>",
    "private_key_file": "<path to private key file matching the certificate sent to other federates>",
    "tmp_dh_file": "<optional file with parameters to use for the key exchange>",
    "password": "<password to decrypt the private key file; default empty>"
}
```

An example of what this might look like after being filled in is:

```json
{
  "encrypted": true,
  "verify_file": "/home/username/openssl_certs/ca.pem",
  "certificate_chain_file": "/home/username/openssl_certs/server.pem",
  "private_key_file": "/home/username/openssl_certs/server.pem",
  "tmp_dh_file": "/home/username/openssl_certs/dh4096.pem",
  "password": "test"
}
```

In an actual set up, it is likely that brokers and federates would have different private key files. A later section in the
documentation has a brief description of how to generate self-signed certificates. It is important to be aware
that the choices you make while generating the certificates and private keys, and how you get them onto individual machines
running the federates can have an impact on the security of communication for the co-simulation. This is not intended as a guide
to those issues, and you should seek outside help in the form of other resources on the topic and from security experts at your
organization.

Search terms to get started are: "Public Key Infrastructure (PKI)", "TLS fundamentals/basics", "secure private key management".
Some of the resources out there (as of early 2023, check archive.org if the site goes offline in the future) include:

- <https://github.com/ssllabs/research/wiki/SSL-and-TLS-Deployment-Best-Practices> covers a good number of the issues for using encryption securely
- <https://opensource.com/article/19/6/cryptography-basics-openssl-part-1> at the section title "The hidden security pieces in the client program"
- <https://geekflare.com/tls-101/> provides a decent overview, though website/browser related parts aren't relevant to HELICS
- <https://www.internetsociety.org/deploy360/tls/basics/> also seems to be a decent source of information

Courses provided at a local university or online can also be a good source of information on best practices, such as:

- <https://www.udemy.com/course/ssl-tls-intro/>
- <https://immersivelabs.online/> "TLS fundamentals" series of labs

### Bridge Encrypted and Unencrypted Communication

A mix of encrypted and unencrypted communication types is possible using the [HELICS Multi Broker](multibroker.md) functionality.
See the linked article for more details on how to set this up.

An example of a configuration file that creates both an encrypted tcpss core and an unencrypted tcpss core is shown below.

```json
{
  "master": {
    "core_type": "test"
  },
  "comms": [
    {
      "core_type": "tcp_ss",
      "local_port": 30000,
      "encrypted": true,
      "encryption_config": "<path to encryption config JSON file>"
    },
    {
      "core_type": "tcp_ss",
      "local_port": 40000
    }
  ]
}
```

Federates using encryption can connect to the broker on port 30000, and federates that don't require encrypted communication
can connect to the broker using port 40000.

## Creating Self-Signed SSL/TLS Certificates

This section assumes you have the `openssl` command installed and working on a Unix based system (Linux, macOS, Windows Subsystem for Linux).

If desired, a `openssl.cnf` file can be used to set some default values and provided to the following commands using the `-config` command
line argument.

The OpenSSL Essentials tutorial from Digital Ocean, which can be found at <https://www.digitalocean.com/community/tutorials/openssl-essentials-working-with-ssl-certificates-private-keys-and-csrs>, gives a good overview of the commands listed here and more.

### Creating a Root CA

The first step to creating certificates is creating a self-signed certificate for our Root CA (Certificate Authority):

```bash
openssl req -new -x509 -keyout root_ca.key -out root_ca.pem
```

This command will ask you for various bits of information, and a password that will be used when creating certificates later. The output will be
a `root_ca.key` file which has the private key for the self-signed Root CA, and a `root_ca.pem` with the public key certificate. It is not necessary
to fill in all of the fields, some may have a default value though can also be left blank.

The `root_ca.pem` file will need to be copied to each machine as a means to verify the signed certificates presented by each of the other machines.

### Creating a certificate for brokers/federates

After creating a Root CA, you can sign certificates for the other machines or brokers/federates that will be connecting to your co-simulation.

First, each machine (or federate/broker) should generate its own public/private key pair using a command such as:

```bash
openssl genrsa -aes128 -out machine1.pem 2048
```

This will ask for a password to encrypt the key, and both keys will be stored in the resulting machine1.key file.

After generating the key pair, a certificate signing request (CSR) needs to be generated, which the Root CA will use to create a signed certificate
that brokers/federates can use to verify that the machine they are connecting to is the one that they expect. The command to create a CSR is:

```bash
openssl req -new -key machine1.pem -out machine1.csr
```

Using the Root CA created in the first step, a signed certificate is created that can be given to the other machines:

```bash
openssl ca -in machine1.csr -out machine1.crt -cert root_ca.pem -keyfile root_ca.key
```

Finally, the private key and the signed certificate file can be combined to form a single file for convenience:

```bash
cat machine1.crt >> machine1.pem
```

### Creating a temporary Diffie-Hellman key exchange parameters file

If desired, the optional file containing temporary Diffie-Hellman key exchange parameters can be created with: `openssl dhparam -out dh4096.pem 4096`

### Create HELICS encryption configuration files

After generating private keys and certificates, encryption configuration files can be created for use by brokers and federates running on machine1.
The `verify_file` would be the `root_ca.pem` file. If the signed machine certificate and private key were combined into a single file,
`certificate_chain_file` and `private_key_file` would both be the `machine1.pem` file from our above example. Otherwise, the certificate chain file
would just be the output certificate from the signing step.

An example of what this might look like after being filled in (assuming the private key for machine1 is "test") would be:

```json
{
  "verify_file": "root_ca.pem",
  "certificate_chain_file": "machine1.pem",
  "private_key_file": "machine1.pem",
  "tmp_dh_file": "dh4096.pem",
  "password": "test"
}
```

The private key generating and certificate signing steps would then repeated for each additional machine that will be joining the co-simulation.

## Debugging Connection Errors

As a first step, disable encryption by removing the `--encrypted` flag when launching the broker and all federates.
If establishing a connection still doesn't work, the issue is with the networking configuration such as wrong ports,
IP addresses, or firewalls.

If the simulation runs successfully with encryption disabled, then the issue is likely either forgetting to enable encryption
on either the broker or one of the federates, or not giving the broker and federates the right set of certificates, CA files,
and private keys. Setting the logging level for the broker and all federates to a higher level such as `debug` or `trace` may
be able to provide more insight into the underlying problem.

To check what certificate the broker server is sending to clients when using an encrypted tcpss core, you can use
`openssl s_client -connect localhost:33133 -showcerts` if you have the `openssl` command/package installed. This
may be useful for confirming that it is sending the certificate that you expect.
