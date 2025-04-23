import 'dart:io';

void main(List<String> args) {
  final files =
      Directory('./src/').listSync()
        ..removeWhere((element) => element.path.contains('main.cpp'));

  final jobs = [
    (
      sourcePath: './src/main.cpp',
      destPath: './croaster-arduino/croaster-arduino.ino',
    ),
    for (final file in files)
      (
        sourcePath: file.path,
        destPath: file.path.replaceAll('src', 'croaster-arduino'),
      ),
  ];

  print("Copying files...");

  for (var job in jobs) {
    moveFile(job.sourcePath, job.destPath);
  }

  exit(0);
}

void moveFile(String sourcePath, String destPath) {
  // Check if source file exists
  final sourceFile = File(sourcePath);
  if (!sourceFile.existsSync()) {
    stderr.writeln("Error: Source file '$sourcePath' does not exist.");
    exit(1);
  }

  // Get the destination directory
  final destDir = Directory(destPath).parent;

  // Check if destination directory exists, create it if not
  if (!destDir.existsSync()) {
    print(
      "Destination directory '${destDir.path}' does not exist. Creating it...",
    );
    try {
      destDir.createSync(recursive: true);
    } catch (e) {
      stderr.writeln(
        "Error: Failed to create destination directory '${destDir.path}': $e",
      );
      exit(1);
    }
  }

  // Copy the file
  try {
    sourceFile.copySync(destPath);
    print("Successfully copied '$sourcePath' to '$destPath'.");
  } catch (e) {
    stderr.writeln("Error: Failed to copy '$sourcePath' to '$destPath': $e");
    exit(1);
  }
}
