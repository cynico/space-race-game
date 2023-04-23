#!/bin/sh

if [ $# -ne 1 ]; then
    echo "Usage: $0 [number-of-req-to-stop-at]";
    echo "Example: $0 9"
    echo "The above would run the requirements up to req 9 (included)"
    exit 1;
fi;

stop=$(( $1 - 1 ))
nameOfRequirements=("shader-test" "mest-test" "transform-test" "pipeline-test" "texture-test" "sampler-test" "material-test" "entity-test" "renderer-test" "sky-test" "postprocess-test")

for req in ${!nameOfRequirements[@]}; do

    # Termination condition.
    if [ $req -gt $stop ]; then
        break;
    fi

    # Running tests under the current requirement.
    for testFile in config/${nameOfRequirements[$req]}/*; do
        ./bin/GAME_APPLICATION -c $testFile -f 1 > /dev/null 2>&1
    done
    
    pwsh -Command ./scripts/compare-all.ps1 -t ${nameOfRequirements[$req]} | grep -qqi mismatch
    
    if [ $? -ne 0 ]; then
        echo "correct outputs of ${nameOfRequirements[$req]}"
    else 
        echo "mismatch in outputs of ${nameOfRequirements[$req]}"
    fi;

done
