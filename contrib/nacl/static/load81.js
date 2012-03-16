$(document).ready(function(){
	function switch_screen(id) {
		$(".screen").css({ "z-index": 50 });
		$(".screen").animate({ opacity:0.0 }, { queue: false, duration: "slow" });
		$("#"+id).css({ "z-index": 60 });
		$("#"+id).animate({ opacity:1.0 }, { queue: false, duration: "slow" });
	}

	/*
	 * Native Client events
	 */

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
			switch_screen("play-screen");
		}
	}

	function handleMessage(message_event) {
		console.log(message_event.data);
	}

	var listener = document.getElementById('play-screen')
	listener.addEventListener('loadstart', moduleDidStartLoad, true);
	listener.addEventListener('progress', moduleLoadProgress, true);
	listener.addEventListener('error', moduleLoadError, true);
	listener.addEventListener('abort', moduleLoadAbort, true);
	listener.addEventListener('load', moduleDidLoad, true);
	listener.addEventListener('loadend', moduleDidEndLoad, true);
	listener.addEventListener('message', handleMessage, true);

	
	/*
	 * Menu buttons
	 */

	$("#play").click(function(event) {
		switch_screen("play-screen");
		document.getElementById('nacl_module').focus();
	});

	$("#help").click(function(event) {
		switch_screen("help-screen");
	});

	$("#load").click(function(event) {
		// TODO
		alert("load!");
	});

	$("#save").click(function(event) {
		// TODO
		alert("save!");
	});
});
