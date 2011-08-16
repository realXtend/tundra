#!/bin/bash
bashtrapExit()
{
    if [ -e jsmodules/startup/$filename ]; then
        rm -f jsmodules/startup/$filename
    ps x | grep '\.\/server --file scenes\/.*--startserver' | awk '{print $1}' | xargs kill
    fi
}
bashtrapKillTerm()
{
    exitCode="$?"
    #This trap is used to clean up possible temp files, etc. if script recieves TERM KILL signal.
    echo 'Program sent exit code: ' $exitCode
    rm -f jsmodules/startup/$filename
    echo 'Killing servers now...'
    ps x | grep '\.\/server --file scenes\/.*--startserver' | awk '{print $1}' | xargs kill
    exit $?
}
# Test if file exists which is given as 1st parameter for this script.
if [ ! -e "$1" ];
then
    echo "File '"$1"' does not exists! Example startup scripts found at:" $PWD/valgrind/startupScripts/
    echo '------------'
    ls -l $PWD/valgrind/startupScripts/ | grep '\.js'
    echo '------------'
else
    trap bashtrapKillTerm INT TERM KILL
    trap bashtrapExit EXIT
    filename=$(basename $1)
    
    # Start two local servers.
    ./server --file scenes/Avatar/avatar.txml --protocol tcp --startserver 2345 &
    ./server --file scenes/DayNight/TestWorld.txml protocol tcp --startserver 2346 &
    
    # Copy desired javascript file to jsmodules/startup/ for viewer to execute.
    cp valgrind/startupScripts/$filename jsmodules/startup/
    
    # Start viewer with valgrind tool memcheck.
     valgrind --tool=memcheck ./viewer --storage scenes/

    # When valgrind run ends, parse logs to separate files in valgrind directory.
    if [ -e "valgrind/logs/valgrindMemcheck.log" ]; then
        cd valgrind/logs/
        cat valgrindMemcheck.log | grep 'are definitely lost' -A12 > definitelyLost.log && cat valgrindMemcheck.log | grep 'LEAK SUMMARY' -A15 -B1 >> definitelyLost.log
        cat valgrindMemcheck.log | grep 'are possibly lost' -A12 > possiblyLost.log && cat valgrindMemcheck.log | grep 'LEAK SUMMARY' -A15 -B1 >> possiblyLost.log
        cd ../../
    fi
    
    # Start viewer with valgrind tool massif.
    valgrind --tool=massif ./viewer --storage scenes/
    
    # Make massif.out human readable and save it to valgrind directory.
    if [ -e "valgrind/logs/massif.out" ]; then
        cd valgrind/logs/
        cp ../base.txt massifExcel.xls
        cat massif.out | grep ^time= -A2 | cut -d= -f2 | tr "\\n" ";" | tr "-" "\\n" | grep ^\; | sed 's/.\(.*\)/\1/' | sed 's/\(.*\)./\1/' >> massifExcel.xls
        ms_print massif.out > massif.log
        cd ../../
    fi
    
    # Print massif graph and leak summary for the user.
    if [ -e "valgrind/logs/massif.log" ]; then
        cat valgrind/logs/massif.log | head -30
    else
        echo 'massif.log not found.'
    fi
    if [ -e "valgrind/logs/valgrindMemcheck.log" ]; then    
        cat valgrind/logs/valgrindMemcheck.log | grep 'LEAK SUMMARY' -A15 -B1
    else
        echo 'valgrindMemcheck.log not found.'
    fi
    
    echo ''
    echo 'Full valgrind logs are stored in '$PWD'/valgrind/logs/'
    echo 'End of valgrind testdrive.'
    echo ''
fi
