
echo "protoc complie begin"
echo ""

# src_dir="./proto/test.proto"
# out_dir="./src/proto/"

# protoc -I ./proto/ -I . --cpp_out=${out_dir} ${src_dir}

# cur_dir=$(pwd)
# retar_dir="proto/"
# src_dir="$cur_dir/$retar_dir"

# tar_dir="src/proto/"
# out_dir="$cur_dir/$tar_dir"

# find "$src_dir" -type f -name "*" | while read file; do
#     echo "found file: $file"
#     local_dir=$file
#     rsync -a --exclude="$file" "$src_file" "$local_dir"
#     echo "rsync file: $local_dir"
# done

echo ""
echo "protoc complie end"