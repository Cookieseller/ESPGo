Building ESPGo
-------------------

In order to build ESPGo on Windows, follow these steps:

1. Download [protobuf-2.5.0.zip](https://github.com/google/protobuf/releases/download/v2.5.0/protobuf-2.5.0.zip) and extract it into the `ESPGo` folder. This creates the folder `ESPGo/protobuf-2.5.0`.
2. Open `ESPGo/protobuf-2.5.0/vsprojects/protobuf.sln` in Microsoft Visual Studio 2010. Allow Visual Studio to convert the projects.
3. Build the *Release* or *Debug* version of `libprotobuf`. Building any other projects is not required.
4. Execute the make_cc_files.bat batch file, this creates the generated_proto folder.
