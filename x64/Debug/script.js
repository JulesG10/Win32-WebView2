
document.addEventListener("DOMContentLoaded", () => {

	console.log(window.chrome.webview)
	window.chrome.webview.addEventListener("message", (e) => {
		console.log(e)
		if (e.data !== null) {
			document.getElementById("msg").innerText = e.data.message;
		}
		if (navigator.geolocation) {
			setInterval(() => {
				navigator.geolocation.getCurrentPosition(position => {
					console.log(position.coords.latitude);
					console.log(position.coords.longitude);
					document.getElementById("msg").innerText = position.coords.latitude + " " + position.coords.longitude;
				});
			}, 1000)
		}
	})
	window.chrome.webview.postMessage({ "message": "Is web message working ?" });


	document.addEventListener("keydown", (e) => {
		if ((e.ctrlKey && (e.key == "f" || e.key == "p" || e.key == "r")) || (e.which == 114 || e.which == 116 || e.which == 123)) {
			e.cancelBubble = true;
			e.preventDefault();
			e.stopImmediatePropagation();
		}
	})

	if (navigator.mediaDevices.getUserMedia) {
		navigator.mediaDevices.getUserMedia({ video: true }).then((stream) => {
			document.getElementById("video").srcObject = stream;
		})
	}
});
