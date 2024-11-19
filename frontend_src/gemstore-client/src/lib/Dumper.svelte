<script module>
	export let ipAddrDefault = "127.0.0.1"
	export let peerPortDefault = 4095
	export let clientPortDefault = 4096
</script>

<script lang="ts">
	import { onDestroy } from "svelte";

	let ipAddr: string       = $state(ipAddrDefault)
	let peerPort: number     = $state(peerPortDefault)
	let clientPort: number   = $state(clientPortDefault)
	let pingDelay: number    = $state(500)
	let pingStarted: boolean = $state(false)
	let responseText: string = $state("")
	let nickname: string = $state("")
	let peers: string = $state("")

	onDestroy(() => {
		pingStarted = false
	})

	const getServInfo = async () => {
		fetch(`http://${ipAddr}:${clientPort}/`)
			.then(resp => resp.json(), reason => responseText = `${reason}`)
			.then(value => {
				if (value == null || value == undefined) {
					nickname = ""
				}
				nickname = value.nickname
				for (let peer of value.connected_peers) {
					peers += `(${peer.address} ${peer.peer_port} ${peer.client_port}) `
				}
			})
	}

	const startPing = async () => {
		pingStarted = true
		getServInfo()
		while (pingStarted) {
			fetch(`http://${ipAddr}:${clientPort}/dump`)
				.then(resp => resp.json(), reason => {
					responseText = `${reason}`;
				})
				.then(value => {
					responseText = JSON.stringify(value, null, 4);
				}, reason => {
					responseText = `${reason}`;
				})
			await new Promise(r => setTimeout(r, pingDelay))
		}
	}
</script>

<div class="box"
	style:padding="0px"
	style:background-color="#cccccc">
	<div class="box">
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
			<button onclick={startPing}>Start</button>
		{:else}
			<button onclick={() => {
				pingStarted = false;
				nickname = "";
			}}>Stop</button>
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
	</div>


	{#if nickname.length > 0}
		<div class="box"
			style:background-color="lightgreen">
			Nickname: {nickname} <br/>
			Peers: {peers}
		</div>
	{/if}

	{#if responseText.length > 0}
		<div class="box"
			style:font-family="monospace"
			style:background-color="white">
			{responseText}
		</div>
	{/if}
</div>
