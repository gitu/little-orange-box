<script setup lang="ts">

import {computed, onMounted, ref} from 'vue'
import {useGamepad} from '@vueuse/core'

const {isSupported, gamepads, onConnected, onDisconnected} = useGamepad();

const gamepad = computed(() => gamepads.value[0])

const connected = ref(false)


onConnected((index) => {
  console.log(`${gamepads.value[index].id} connected`)
  console.log(`${gamepads.value[index].mapping} connected`)
  console.log(gamepads.value[index])
  connected.value = true
})

onDisconnected((index) => {
  console.log(`${index} disconnected`)
  connected.value = false
})

onMounted(() => {
  console.log('mounted')
  setInterval(() => {
    console.log(gamepad.value?.buttons[0].pressed + " " + gamepad.value?.buttons[1].touched)
  }, 1000)
});

</script>

<template>
  <div v-if="!isSupported" flex="~ row" place="items-center content-center" items="center" space="x-4">
    <i i-carbon-error text="5xl" opacity="50"/>
    <div flex="~ col">
      <span text="2xl">Gamepad is not supported on this device.</span>
      <span opacity="70">It seems your device does not support the Gamepad API. Check <a
          href="https://caniuse.com/gamepad">here</a> for a list supported devices.</span>
    </div>
  </div>
  <div v-else-if="gamepads.length === 0" flex="~ row" place="items-center content-center" items="center" space="x-4">
    <i i-carbon-game-console text="5xl" opacity="50"/>
    <div flex="~ col">
      <span text="2xl">No Gamepad Detected</span>
      <span opacity="50">Ensure your gamepad is connected and press a button to wake it up.</span>
    </div>
  </div>
  <div v-else space="y-4">
    {{gamepads[0]}}
  </div>
</template>

<style scoped>
.read-the-docs {
  color: #888;
}


.logo {
  height: 6em;
  padding: 1.5em;
  will-change: filter;
  transition: filter 300ms;
}

.logo:hover {
  filter: drop-shadow(0 0 2em #646cffaa);
}

.logo.vue:hover {
  filter: drop-shadow(0 0 2em #42b883aa);
}
</style>
