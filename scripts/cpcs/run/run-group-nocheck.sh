mpirun -np "$2" build/mallob -v=4 -c=1 -ajpc="$1" -J="$1" \
  -job-desc-template=scripts/cpcs/input/instance_file.txt \
  -job-template=scripts/cpcs/input/job-group-nocheck.json \
  -client-template=templates/client-template.json \
  | grep -i "cpcs\|exited\|error\|warn\|solution\|over-due\|RESPONSE_TIME\|idle"

pkill mallob
