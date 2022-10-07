
cmake_minimum_required (VERSION 3.7.2)

project (Test_project)

find_library (EV_LIBRARY        ev)
find_library (OPENSSL_LIBRARY   crypto)
find_library (PTHREAD_LIBRARY   pthread)

include_directories (.)

add_executable (sent        Sent.c       )
add_executable (received    Received.c   Processed.c  ring_buffer.c )

target_link_libraries (sent     ${PTHREAD_LIBRARY} ${EV_LIBRARY} ${OPENSSL_LIBRARY})
target_link_libraries (received ${PTHREAD_LIBRARY} ${EV_LIBRARY} ${OPENSSL_LIBRARY})
