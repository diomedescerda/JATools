for f in *.mkv; do
    mkvmerge -o "${f%.mkv}_TEMP.mkv" --audio-tracks 2 --no-subtitles "$f"
    
    if [ -f "${f%.mkv}_TEMP.mkv" ]; then
        rm "$f"
        mv "${f%.mkv}_TEMP.mkv" "$f"
        echo "Successfully replaced: $f"
    else
        echo "Error processing $f - original file remains"
    fi
done
