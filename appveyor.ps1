# Reference: https://www.appveyor.com/docs/running-tests/
# Make the results of the unit testing available as artifacts
# upload results to AppVeyor
$wc = New-Object 'System.Net.WebClient'
$wc.UploadFile("https://ci.appveyor.com/api/testresults/junit/$($env:APPVEYOR_JOB_ID)", (Resolve-Path .\unittest.xml))
