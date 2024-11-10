<script module>
	export let ipAddrDefault = "127.0.0.1"
	export let peerPortDefault = 4095
	export let clientPortDefault = 4096
</script>

<script lang="ts">
	import { onDestroy } from "svelte";

	let form: HTMLFormElement;
	let ipAddr: string       = $state(ipAddrDefault)
	let peerPort: number     = $state(peerPortDefault)
	let clientPort: number   = $state(clientPortDefault)
	let key: string          = $state("")
	let responseText: string = $state("")

	const getValue = async () => {
		let params = new URLSearchParams({ "q": key })
		fetch(`http://${ipAddr}:${clientPort}/query?${params.toString()}`)
			.then(resp => resp.json(), reason => responseText = `${reason}`)
			.then(value => responseText = JSON.stringify(value, null, 4))
	}

</script>

<div class="box"
	style:padding="0px"
	style:background-color="#cccccc">
	<form class="box" bind:this={form} onsubmit={(e) => {
		e.preventDefault();
		if (form.checkValidity()) {
			getValue();
		}
	} }>
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
			Key:
			<input type="text" bind:value={key} style:width="4em" required/>
		</div>

		<div class="inputelem">
		<input type="submit" value="Get" />
		</div>
	</form>

	{#if responseText.length > 0}
	<div class="box"
		style:font-family="monospace"
		style:background-color="white">
		{responseText}
	</div>
	{/if}
</div>
