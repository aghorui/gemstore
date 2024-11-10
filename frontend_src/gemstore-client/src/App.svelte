<script lang="ts">
	import Dumper from "./lib/Dumper.svelte";
	import GetValue from "./lib/GetValue.svelte";
	import SetValue from "./lib/SetValue.svelte";
	let numDumpers: number = $state(1)
</script>

<main>

<div class="container">

<div class="header-title">
<span class="linear-text-gradient"> Gemstore </span> Testing Webclient
</div>

<div class="header-rule"> </div>

<div class="filter-body">
	<h1>Dumpers</h1>
	<div class="box">
		<div class="inputelem">
			<button onclick={() => numDumpers += 1}>Add</button>
		</div>
		<div class="inputelem">
			<button onclick={() => numDumpers = 1}>Clear</button>
		</div>
	</div>

	{#each {length: numDumpers} as _, i}
		<div style:height="10px"></div>
		<Dumper />
	{/each}


	<div style:height="40px"></div>
	<h1>Get a Value</h1>
	<GetValue />

	<div style:height="40px"></div>
	<h1>Set a Value</h1>
	<SetValue />
</div>

</div>

</main>

<style>

</style>
