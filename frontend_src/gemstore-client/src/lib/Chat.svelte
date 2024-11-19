<script module>
	export let ipAddrDefault = "127.0.0.1"
	export let peerPortDefault = 4095
	export let clientPortDefault = 4096
</script>

<script lang="ts">
	import { onDestroy } from "svelte";
	import { writable } from "svelte/store";

	let connect_form: HTMLFormElement;
	let msg_form: HTMLFormElement;
	let ipAddr: string       = $state(ipAddrDefault)
	let peerPort: number     = $state(peerPortDefault)
	let clientPort: number   = $state(clientPortDefault)
	let pingDelay: number    = $state(500)
	let pingStarted: boolean = $state(false)
	let responseText: string = $state("")
	let username: string = $state("")
	let message: string = $state("")
	// Username, timestamp, text
	let messagedata = writable<Array<[number, string, string]>>([]);

	onDestroy(() => {
		pingStarted = false
	})

	let resp: Promise<Response> | null = $state(null);

	const unixTimestampToDate = (timestampMs: number) => {
		var date = new Date(timestampMs);
		var hours = date.getHours();
		var minutes = "0" + date.getMinutes();
		var seconds = "0" + date.getSeconds();
		return hours + ':' + minutes.substr(-2) + ':' + seconds.substr(-2);
	}

	let startPing = async () => {
		pingStarted = true
		while (pingStarted) {
			let params = new URLSearchParams({ "q": "ul" })
			fetch(`http://${ipAddr}:${clientPort}/query?${params.toString()}`)
				.then(resp => {
					if (resp.status == 200) {
						responseText = ``
						return resp.json()
					} else if (resp.status == 404) {
						console.log("setting messagetext 404")
					} else {
						responseText = `${resp}`
					}
				}, reason => {
					console.log(reason)
					responseText = `${reason}`
				}).then(
				data => messagedata.set(data.value),
				reason => {
					console.log(reason)
					responseText = `${reason}`
				})
			await new Promise(r => setTimeout(r, pingDelay))
		}
	}

	// let startPing = async () => {
	// 	pingStarted = true
	// 	while (pingStarted) {
	// 		let params = new URLSearchParams({ "q": "ul" })
	// 		resp = fetch(`http://${ipAddr}:${clientPort}/query?${params.toString()}`)
	// 		await new Promise(r => setTimeout(r, pingDelay))
	// 	}
	// }


	let postMessage = async () => {
		fetch(`http://${ipAddr}:${clientPort}/set`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json"
			},
			body: JSON.stringify({
				"key": "ul",
				"value": [
					[Date.now(),
					username,
					message]
				]
			})
		})
	}

	
</script>

<div class="box"
	style:padding="0px"
	style:background-color="#cccccc">

	<form class="box" bind:this={connect_form} onsubmit={(e) => {
		e.preventDefault();
		if (connect_form.checkValidity()) {
			startPing();
		}
	} }>
		<div class="inputelem">
			Address:
			<input type="text" bind:value={ipAddr} disabled={pingStarted} required/>
		</div>

		<div class="inputelem">
			Peer Port:
			<input type="number" bind:value={peerPort} disabled={pingStarted} style:width="6em" required/>
		</div>

		<div class="inputelem">
			Client Port:
			<input type="number" bind:value={clientPort} disabled={pingStarted} style:width="6em" required/>
		</div>

		<div class="inputelem">
			Ping Delay:
			<input type="number" bind:value={pingDelay} disabled={pingStarted} style:width="4em" required/>
		</div>

		<div class="inputelem">
		{#if !pingStarted}
			<input type="submit" value="Start" />
		{:else}
			<button onclick={() => pingStarted = false}>Stop</button>
		{/if}
		</div>

		<div class="inputelem">
		{#if pingStarted}
			<b
				style:background-color="green"
				style:color="white"
				style:padding="5px"
				style:padding-left="10px"
				style:padding-right="10px"
				style:border-radius="3px"
				style:font-size="small">
			Running</b>
		{/if}
		</div>
	</form>

{#if responseText.length > 0}
	<div class="box"
		style:font-family="monospace"
		style:background-color="white">
		{responseText}
	</div>
{/if}

{#if pingStarted}

	<form class="box" bind:this={msg_form} onsubmit={(e) => {
		e.preventDefault();
		if (msg_form.checkValidity()) {
			postMessage();
			message = ""
		}
	} }>
		<div class="inputelem" style="display: flex; flex-shrink: 1; flex-direction: row; gap: 10px">
			<div> Username: </div>
			<input type="text" bind:value={username} required/>
		</div>
		<div class="inputelem" style="display:flex; gap: 10px; margin: 5px 0 5px 0; flex-direction: row;">
			<div>Message:</div>
			<input style:flex="1" type="text" bind:value={message} required/>
			<input type="submit" value="Post" />
		</div>
	</form>

	<h4 class="box" style="background-color: #D5F3FF; padding: 5px;">Chatbox:</h4>
	<div class="box" style="background-color:white; overflow: auto; max-height: 500px; height: 500px;">
		{#each $messagedata as msg}
			<div>
				<b>{unixTimestampToDate(msg[0])} {msg[1]}:</b> {msg[2]}
			</div>
		{/each}
	</div>
{/if}
</div>
