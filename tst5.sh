time ./build/microbench \
--keys_file=../datasets/planet \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=0 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=0 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/osm \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=0 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/fb \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=0 \
--table_size=-1 \
--init_table_ratio=1 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/planet \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=100000000 \
--table_size=-1 \
--init_table_ratio=0.5 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/covid \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=100000000 \
--table_size=-1 \
--init_table_ratio=0.5 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/osm \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=100000000 \
--table_size=-1 \
--init_table_ratio=0.5 \
--thread_num=1 \
--index=lipp --lyc_debug \

time ./build/microbench \
--keys_file=../datasets/fb \
--keys_file_type=binary \
--read=0 --insert=1 \
--operations_num=100000000 \
--table_size=-1 \
--init_table_ratio=0.5 \
--thread_num=1 \
--index=lipp --lyc_debug \