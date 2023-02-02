mpirun -np 2 build/mallob -v=4 -c=1 -ajpc=2 -ljpc=4 -J=2 \
-job-desc-template=scripts/cpcs/input/instance_file.txt \
-job-template=scripts/cpcs/input/job-template.json \
-client-template=scripts/cpcs/input/client-template.json -pls=0 \
| grep -i "cpcs\|exited\|error\|warn\|solution"