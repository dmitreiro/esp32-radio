// HTML content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Volume and Radio</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            padding: 10px;
            box-sizing: border-box;
            background-color: #121212;
            color: white;
        }
        .slider-container, .radio-container {
            text-align: center;
            margin-top: 20px;
            width: 100%;
        }
        .slider {
            width: 100%;
            max-width: 300px;
        }
        .value {
            margin-top: 10px;
            font-size: 1.2em;
        }
        .radio-buttons button, .custom-stream button {
            margin: 5px;
            padding: 10px 20px;
            font-size: 1em;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            background-color: #333;
            color: white;
            transition: background-color 0.3s, transform 0.2s;
        }
        .radio-buttons button.active {
            background-color: green;
            color: white;
        }
        .radio-buttons button:hover, .custom-stream button:hover {
            background-color: #444;
        }
        .radio-buttons button:active, .custom-stream button:active {
            transform: scale(0.95);
        }
        .custom-stream {
            display: flex;
            justify-content: center;
            align-items: center;
            margin-top: 10px;
            width: 100%;
        }
        .custom-stream input {
            width: calc(100% - 95px); /* Adjusted for alignment with buttons */
            max-width: 300px;
            padding: 10px;
            font-size: 1em;
            border: 1px solid #333;
            border-radius: 5px;
            background-color: #333;
            color: white;
            margin-right: 10px;
        }
        .custom-stream button {
            width: 80px;
            padding: 10px;
            font-size: 1em;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            background-color: #333;
            color: white;
            transition: background-color 0.3s, transform 0.2s;
        }
        h2 {
            font-size: 2em; /* Adjusted font size for consistency with Volume */
        }
    </style>
</head>
<body>
    <div class="slider-container">
        <h1>Volume</h1>
        <input type="range" min="0" max="100" value="20" class="slider" id="mySlider">
        <div class="value"><span id="sliderValue">50</span>%</div>
    </div>
    <div class="radio-container">
        <h2>Radio</h2>
        <div class="radio-buttons">
            <button id="radio1Btn" onclick="setStream('radio1')">Radio 1</button>
            <button id="radio2Btn" onclick="setStream('radio2')">Radio 2</button>
            <button id="radio3Btn" onclick="setStream('radio3')">Radio 3</button>
            <button id="radio4Btn" onclick="setStream('radio4')">Radio 4</button>
        </div>
        <div class="custom-stream">
            <input type="text" id="customStream" placeholder="Enter custom stream link">
            <button onclick="applyCustomStream()">Play</button>
        </div>
    </div>

    <script>
        const slider = document.getElementById('mySlider');
        const output = document.getElementById('sliderValue');

        const buttons = {
            radio1: document.getElementById('radio1Btn'),
            radio2: document.getElementById('radio2Btn'),
            radio3: document.getElementById('radio3Btn'),
            radio4: document.getElementById('radio4Btn')
        };

        // Function to get the current volume from the server
        async function getVolume() {
            const response = await fetch('/getVolume');
            const data = await response.json();
            return data.volume;
        }

        // Function to set the volume on the server
        async function setVolume(value) {
            await fetch(`/setVolume?value=${value}`);
        }

        // Function to set the radio stream on the server
        async function setStream(stream) {
            await fetch(`/setStream?radio=${stream}`);
            updateActiveButton(stream);
        }

        // Function to apply custom stream
        async function applyCustomStream() {
            const customStream = document.getElementById('customStream').value;
            await fetch(`/setCustomStream?link=${encodeURIComponent(customStream)}`);
            updateActiveButton(null);
        }

        // Function to update the active button style
        function updateActiveButton(activeStream) {
            for (const key in buttons) {
                if (key === activeStream) {
                    buttons[key].classList.add('active');
                } else {
                    buttons[key].classList.remove('active');
                }
            }
        }

        // Initialize slider with current volume
        getVolume().then(volume => {
            slider.value = volume;
            output.innerHTML = volume;
        });

        slider.oninput = function() {
            output.innerHTML = this.value;
            setVolume(this.value);
        }

        // Fetch the current stream from the server on load
        async function getCurrentStream() {
            const response = await fetch('/getCurrentStream');
            const data = await response.json();
            updateActiveButton(data.currentStream);
        }

        // Initialize the current stream on page load
        getCurrentStream();
    </script>
</body>
</html>
)rawliteral";