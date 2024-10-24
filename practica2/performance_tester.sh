#!/bin/bash

# Parameters
num_runs=5           # Number of runs to calculate mean times
threads=(1 2 10 30)  # Thread counts to test
methods=(1 2 3 4)    # Methods to test
file="./numeros.txt" # File to process

# Start timer for the entire script
script_start=$(date +%s.%N)

# Initialize an associative array to store results
declare -A results

# Loop over methods and thread counts
for method in "${methods[@]}"; do
    for thread in "${threads[@]}"; do
        total_external_time=0
        total_internal_time=0
        echo "Testing method $method with $thread threads..."

        # Run the program multiple times to compute mean execution times
        for ((i = 1; i <= num_runs; i++)); do
            # Measure external execution time
            start=$(date +%s.%N)
            output=$(./sum "$file" "$thread" "$method")
            end=$(date +%s.%N)

            # Calculate external time (real execution time) with floating point precision
            external_time=$(echo "scale=6; $end - $start" | bc)

            # Extract internal execution time from program's output
            internal_time=$(echo "$output" | grep -oP 'en tiempo \K[0-9.]+')

            # Add both to their respective totals
            total_external_time=$(echo "scale=6; $total_external_time + $external_time" | bc)
            total_internal_time=$(echo "scale=6; $total_internal_time + $internal_time" | bc)
        done

        # Calculate mean external and internal times
        mean_external_time=$(echo "scale=6; $total_external_time / $num_runs" | bc)
        mean_internal_time=$(echo "scale=6; $total_internal_time / $num_runs" | bc)

        # Store results in the associative array
        key="${method}-${thread}"
        results["$key"]="$mean_external_time,$mean_internal_time"
    done
done

# End timer for the entire script
script_end=$(date +%s.%N)
total_script_time=$(echo "scale=6; $script_end - $script_start" | bc)

# Print total execution time for the script
echo "\nTotal Execution Time for the script: $total_script_time seconds"

# Print final table with all results
echo ""
echo "Final Results Table:"
echo "----------------------------------------------------------------------"
echo "| Method | Threads | Mean External Time (s) | Mean Internal Time (s) |"
echo "----------------------------------------------------------------------"
for key in $(for k in "${!results[@]}"; do echo "$k"; done | sort -t- -k1,1n -k2,2n); do
    method=$(echo "$key" | awk -F- '{print $1}')
    threads=$(echo "$key" | awk -F- '{print $2}')
    mean_external_time=$(echo "${results[$key]}" | awk -F, '{print $1}')
    mean_internal_time=$(echo "${results[$key]}" | awk -F, '{print $2}')
    printf "| %-6s | %-7s | %-22s | %-22s |\n" "$method" "$threads" "$mean_external_time" "$mean_internal_time"
done
echo "----------------------------------------------------------------------"

# Print table ordered by internal time asc
echo ""
echo "Results Ordered by Internal Time (Ascending):"
echo "----------------------------------------------------------------------"
echo "| Method | Threads | Mean External Time (s) | Mean Internal Time (s) |"
echo "----------------------------------------------------------------------"
for key in $(for k in "${!results[@]}"; do echo "$k,${results[$k]}"; done | sort -t, -k3,3n | awk -F, '{print $1}'); do
    method=$(echo "$key" | awk -F- '{print $1}')
    threads=$(echo "$key" | awk -F- '{print $2}')
    mean_external_time=$(echo "${results[$key]}" | awk -F, '{print $1}')
    mean_internal_time=$(echo "${results[$key]}" | awk -F, '{print $2}')
    printf "| %-6s | %-7s | %-22s | %-22s |\n" "$method" "$threads" "$mean_external_time" "$mean_internal_time"
done
echo "----------------------------------------------------------------------"

# Find the best result based on internal time
best_result=""
best_internal_time=""

for key in "${!results[@]}"; do
    mean_internal_time=$(echo "${results[$key]}" | awk -F, '{print $2}')
    if [[ -z "$best_internal_time" || $(echo "$mean_internal_time < $best_internal_time" | bc) -eq 1 ]]; then
        best_internal_time=$mean_internal_time
        best_result=$key
    fi
done

# Print the best result
echo ""
echo "Best Result Based on Internal Time:"
echo "----------------------------------------------------------------------"
echo "| Method | Threads | Mean External Time (s) | Mean Internal Time (s) |"
echo "----------------------------------------------------------------------"
method=$(echo "$best_result" | awk -F- '{print $1}')
threads=$(echo "$best_result" | awk -F- '{print $2}')
mean_external_time=$(echo "${results[$best_result]}" | awk -F, '{print $1}')
mean_internal_time=$(echo "${results[$best_result]}" | awk -F, '{print $2}')
printf "| %-6s | %-7s | %-22s | %-22s |\n" "$method" "$threads" "$mean_external_time" "$mean_internal_time"
echo "----------------------------------------------------------------------"
