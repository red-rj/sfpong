# helper para chamar robocopy
$prefix = '[Robocopy]'
$ec_messages = @{
    0='No changes'
    1='Copy sucessfull'
    2='Extra files or directories detected'
    4='Mismatched files or directories detected'
    8='Some items could not be copied'
    16='FATAL ERROR'
}

function errormsg ($code) {
    $msg = $ec_messages.Keys | foreach {
        $c = $code -band $_
        if ($c) {
            echo $ec_messages[$c]
        }
    } | join-string -sep '; '

    echo $msg
}

# exec
Robocopy.exe $args
$exitcode = $LASTEXITCODE
$rcemsg = errormsg $exitcode

if ($exitcode -ge 8) {
    # erro
    Write-Error "$prefix error: $rcemsg" -ErrorId $exitcode
    exit 1
}
elseif ($rcemsg) {
    Write-Information "${prefix}: $rcemsg"
}

exit 0