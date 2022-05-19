# Integration Tests


Run through ```integration_test.sh```

Be sure to add the corresponding "${test_name}_test" executable in CMakeLists.txt


A test fails if an assertion fails. Otherwise the test passes. So be sure to use assertions in your test main to test what you want to test. Also add try to add more assertions in the src to increase our code's assertion coverage.
