const char htmlPage[] PROGMEM = R"rawLiteral(
<!DOCTYPE html>
<html lang=en>
<head>
    <meta charset=UTF-8/>
    <meta name=viewport content="width=device-width, initial-scale=1.0"/>
    <title>iot monitoring</title>
    <script src=https://cdn.tailwindcss.com></script>
    <style>table {
        width: 100%
    }

    td {
        padding: 6px
    }

    table tr td:nth-child(1) {
        text-align: left
    }

    table tr td:nth-child(2) {
        text-align: right
    }</style>
</head>
<body>
<div class="shadow-[0_20px_50px_rgba(8,_112,_184,_0.7)] w-[320px] h-[220px] mx-auto rounded-xl mt-4">
    <div id=connecting class="w-full h-full flex items-center justify-center">
        <h3 class=text-lg>Connecting ...</h3>
    </div>
    <div id=connected class="px-8 py-4 w-full h-full hidden">
        <div class="font-bold text-xl mb-4 text-[rgba(8,_112,_184,_1)] text-center">
            Monitoring Details
        </div>
        <table>
            <tr>
                <td>Humidity :</td>
                <td id=humidity>%</td>
            </tr>
            <tr>
                <td>Temperature :</td>
                <td id=temperature>°C</td>
            </tr>
            <tr>
                <td>Air Quality :</td>
                <td id=air>ppm</td>
            </tr>
            <tr>
                <td>AQI Status :</td>
                <td id=status>air</td>
            </tr>
        </table>
    </div>
</div>
<p id=value class="mt-12 text-center"></p>
<script>
    const socketUrl = `ws://${window.location.host}:81`
    let socket = null

    function connectWebSocket() {
        socket = new WebSocket(socketUrl)

        socket.onopen = () => {
            console.log('WebSocket connection established')
        }

        socket.onerror = (err) => {
            console.log('WebSocket error:', err)
        }

        socket.onmessage = (message) => {
            try {
                const data = JSON.parse(message.data)
                console.log(data)

                let {humidity, temperature, air, status} = data

                document.getElementById('connected').classList.remove('hidden')
                document.getElementById('connecting').classList.add('hidden')

                document.getElementById('humidity').innerHTML = humidity
                document.getElementById('temperature').innerHTML = temperature
                document.getElementById('air').innerHTML = air
                document.getElementById('status').innerHTML = status

                if (status === "Danger!") document.getElementById('status').style.color = "red"
                else if (status === "Bad Air") document.getElementById('status').style.color = "orange"
                else if (status === "Good Air") document.getElementById('status').style.color = "green"
            } catch (error) {
                console.log(error)
                document.getElementById('connecting').classList.remove('hidden')
                document.getElementById('connected').classList.add('hidden')
            }
        }

        socket.onclose = (event) => {
            console.log('WebSocket connection closed:', event.code, event.reason)
            if (!event.wasClean) {
                console.error('Connection lost unexpectedly')
                document.getElementById('connecting').classList.remove('hidden')
                document.getElementById('connected').classList.add('hidden')
                reconnectWebSocket()
            }
        }

    }

    function reconnectWebSocket() {
        setTimeout(() => {
            console.log('Attempting to reconnect...')
            connectWebSocket()
        }, 2000)
    }

    connectWebSocket()
</script>
</body>
</html>
)rawLiteral";