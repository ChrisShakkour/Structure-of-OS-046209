# Grade: 76
# Errors:
1. bg twice in a row does not work (when there are two stopped processes) (-5)
2. quit kill does not send SIGKILL and keeps processes alive (-5)
3. built-in commands with '&' are ignored (-3)
4. cd with an invalid path does not print an error (-3)
5. incorrect job-id assignment (-5)
6. seconds elapsed is not updated for stopped jobs (-3)
