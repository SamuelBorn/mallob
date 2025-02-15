mpirun -np "$2" build/mallob -v=4 -c=1 -ajpc="$1" -J="$1" -ecct=0 \
  -job-desc-template=scripts/cpcs/input/instance_file.txt \
  -job-template=scripts/cpcs/input/job-group-check.json \
  -client-template=templates/client-template.json \
  | grep -i "cpcs\|exited\|error\|warn\|solution\|over-due\|RESPONSE_TIME\|am idle"

pkill mallob


# exclusive mpirun -np 16 --bind-to core build/mallob -v=4 -c=1 -ajpc=3 -J=1 -job-desc-template=scripts/cpcs/input/instance_file_4.txt -job-template=scripts/cpcs/input/job-group-check.json -client-template=templates/client-template.json | grep -i "cpcs\|exited\|error\|warn\|solution\|over-due\|RESPONSE_TIME\|am idle"
