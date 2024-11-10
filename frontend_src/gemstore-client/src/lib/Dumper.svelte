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

	onDestroy(() => {
		pingStarted = false
	})

	const startPing = async () => {
		pingStarted = true
		while (pingStarted) {
			fetch(`http://${ipAddr}:${clientPort}/dump`)
				.then(resp => resp.json(), reason => responseText = `${reason}`)
				.then(value => responseText = JSON.stringify(value, null, 4))
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
			<input type="text" bind:value={ipAddr} required/>
		</div>

		<div class="inputelem">
			Peer Port:
			<input type="number" bind:value={peerPort} style:width="6em" required/>
		</div>

		<div class="inputelem">
			Client Port:
			<input type="number" bind:value={clientPort} style:width="6em" required/>
		</div>

		<div class="inputelem">
			Ping Delay:
			<input type="number" bind:value={pingDelay} style:width="4em" required/>
		</div>

		<div class="inputelem">
		{#if !pingStarted}
			<button onclick={startPing}>Start</button>
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
	</div>

	{#if responseText.length > 0}
	<div class="box"
		style:font-family="monospace"
		style:background-color="white">
		{responseText}
	</div>
	{/if}
</div>
