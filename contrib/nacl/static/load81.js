$(document).ready(function(){
	function moduleDidStartLoad() {
	}

	function moduleLoadProgress(event) {
		if (event.total != 0) {
			var load_percent = Math.round(100.0 * event.loaded / event.total);
			$("#status").html("Loading: " + load_percent + "%");
		}
	}

	function moduleLoadError() {
	}

	function moduleLoadAbort() {
	}

	function moduleDidLoad() {
		document.getElementById('nacl_module').focus();
	}

	function moduleDidEndLoad() {
		var lastError = event.target.lastError;
		if (lastError != undefined && lastError != "") {
			$("#status").html(lastError)
		} else {
			$("#status").fadeOut("slow")
			$("#nacl_module").animate({ opacity:1.0 }, { queue: false, duration: "slow" })
		}
	}

	function handleMessage(message_event) {
		console.log(message_event.data);
	}

	var listener = document.getElementById('listener')
	listener.addEventListener('loadstart', moduleDidStartLoad, true);
	listener.addEventListener('progress', moduleLoadProgress, true);
	listener.addEventListener('error', moduleLoadError, true);
	listener.addEventListener('abort', moduleLoadAbort, true);
	listener.addEventListener('load', moduleDidLoad, true);
	listener.addEventListener('loadend', moduleDidEndLoad, true);
	listener.addEventListener('message', handleMessage, true);
});
