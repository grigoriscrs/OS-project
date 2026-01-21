
./scheduler_v1 FCFS_PNP priority_test.txt > v1_pnp_output.txt
./scheduler_v1 FCFS_PRIO priority_test.txt > v1_prio_output.txt

./scheduler_v2 FCFS mixed.txt > v2_fcfs_mixed_output.txt
./scheduler_v2 RR 1000 mixed.txt > v2_rr_mixed_output.txt

echo "test done, results in txt files"