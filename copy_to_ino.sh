#!/bin/bash

# Function to move/copy file
move_file() {
    local source_path="$1"
    local dest_path="$2"

    # Check if source file exists
    if [ ! -f "$source_path" ]; then
        echo "Error: Source file '$source_path' does not exist." >&2
        exit 1
    fi

    # Get the destination directory
    local dest_dir=$(dirname "$dest_path")

    # Check if destination directory exists, create it if not
    if [ ! -d "$dest_dir" ]; then
        echo "Destination directory '$dest_dir' does not exist. Creating it..."
        if ! mkdir -p "$dest_dir"; then
            echo "Error: Failed to create destination directory '$dest_dir'" >&2
            exit 1
        fi
    fi

    # Copy the file
    if cp "$source_path" "$dest_path"; then
        echo "Successfully copied '$source_path' to '$dest_path'."
    else
        echo "Error: Failed to copy '$source_path' to '$dest_path'" >&2
        exit 1
    fi
}

# Main script
echo "Copying files..."

# Process main.cpp (the reference example sketch)
move_file "./examples/reference/src/main.cpp" "./croaster-arduino/croaster-arduino.ino"

# Process other files in the reference example src directory, excluding main.cpp
for file in ./examples/reference/src/*; do
    if [ "$file" != "./examples/reference/src/main.cpp" ] && [ -f "$file" ]; then
        dest_path=$(echo "$file" | sed 's|examples/reference/src|croaster-arduino|')
        move_file "$file" "$dest_path"
    fi
done

# Copy the library headers/sources used by the sketch into the Arduino sketch
# folder (Arduino IDE needs every source in the sketch folder).
for file in ./src/*; do
    if [ -f "$file" ]; then
        move_file "$file" "./croaster-arduino/$(basename "$file")"
    fi
done

exit 0