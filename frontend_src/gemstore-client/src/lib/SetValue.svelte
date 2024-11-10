<script module>
	export let ipAddrDefault = "127.0.0.1"
	export let peerPortDefault = 4095
	export let clientPortDefault = 4096
</script>

<script lang="ts">
	import { onDestroy } from "svelte";

	let form: HTMLFormElement
	let ipAddr: string       = $state(ipAddrDefault)
	let peerPort: number     = $state(peerPortDefault)
	let clientPort: number   = $state(clientPortDefault)
	let key: string          = $state("")
	let value: any           = $state(null)
	let responseText: string = $state("")

	const setValue = async () => {
		let val_obj = null

		try {
			val_obj = JSON.parse(value)
		} catch (e) {
			responseText = `${e}`
			return;
		}

		fetch(`http://${ipAddr}:${clientPort}/set`, {
			method: "POST",
			headers: {
				"Content-Type": "application/json"
			},
			body: JSON.stringify({
				"key": key,
				"value": val_obj
			})
		})
			.then(resp => responseText = `Success`, reason => responseText = `${reason}`)
	}
</script>

<div class="box"
	style:padding="0px"
	style:background-color="#cccccc">
	<form class="box" bind:this={form} onsubmit={(e) => {
		e.preventDefault();
		if (form.checkValidity()) {
			setValue();
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
			Value:
			<input type="text" bind:value={value} style:width="4em" required/>
		</div>

		<div class="inputelem">
		<input type="submit" value="Set" />
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
