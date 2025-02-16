time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=1 --insert=0 \
--operations_num=200000000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=1024 \
--operations_num=200000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=512 \
--operations_num=400000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=256 \
--operations_num=800000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=128 \
--operations_num=1600000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=64 \
--operations_num=3200000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=32 \
--operations_num=6400000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=16 \
--operations_num=12800000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=8 \
--operations_num=25600000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree


time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=0 --scan=1 --scan_num=4 \
--operations_num=51200000 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=btree
