This has been touched up by Ray Jones to deal with a couple of issues:

bad return from execHTTPcheck()
execHTTPcheck() uses a char[] with a variable 

Examples have been detonated to avoid issues during BTC recompile

Have also added callbacks for:
bool onComplete() - upload is completed but not verified - add own checks here
void onSuccess() - uploaded and verified OK
void onFail() - upload failed
