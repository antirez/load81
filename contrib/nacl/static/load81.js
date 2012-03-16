demo_src = 'function draw() background(100, 50, 50) end'

$(document).ready(function(){
	var nacl_module = null;
	var src = null;

	function main() {
		start_game(demo_src);
	}

	function switch_screen(id) {
		$(".screen").css({ "z-index": 50 });
		$(".screen").animate({ opacity:0.0 }, { queue: false, duration: "slow" });
		$("#"+id).css({ "z-index": 60 });
		$("#"+id).animate({ opacity:1.0 }, { queue: false, duration: "slow" });
	}

	function start_game(_src) {
		if (nacl_module) {
			$(nacl_module).remove();
			nacl_module = null;
		}

		nacl_html = '<embed name="nacl_module"        \
		                    id="nacl_module"          \
		                    src="static/load81.nmf"   \
		                    type="application/x-nacl" \
		                    tabindex="-1" />';

		var e = document.createElement("embed");
		e.id = "nacl_module";
		e.name = "nacl_module";
		e.src = "static/load81.nmf";
		e.type="application/x-nacl";
		$("#play-screen").append(e);
		nacl_module = e;
		src = _src;
	}

	/*
	 * Native Client events
	 */

	function moduleDidStartLoad() {
		$("#status").html("Loading...");
		$("#status").show();
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
		nacl_module.postMessage(src);
		switch_screen("play-screen");
		nacl_module.focus();
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
		if (nacl_module) {
			nacl_module.focus();
		}
	});

	$("#help").click(function(event) {
		switch_screen("help-screen");
	});

	$("#load").click(function(event) {
		switch_screen("load-screen");
	});

	$("#save").click(function(event) {
		// TODO
		alert("save!");
	});


	/*
	 * Load screen
	 */

	$("#uploader").change(function(event) {
		var file = event.target.files[0];
		var reader = new FileReader();
		reader.onload = function(evt) {
			console.log(["uploaded", evt]);
			var code = reader.result;
			start_game(code);
		};  
		reader.readAsBinaryString(file);
	});


	main();
});
