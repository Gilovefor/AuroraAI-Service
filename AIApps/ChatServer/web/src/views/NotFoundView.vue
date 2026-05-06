<script setup>
import { useRouter } from 'vue-router'
const router = useRouter()
</script>

<template>
  <main class="page-shell notfound-wrap">
    <section class="card box">
      <h1>404</h1>
      <p class="muted">Page not found</p>
      <button class="button" @click="router.push('/')">Back to Home</button>
    </section>
  </main>
</template>

<style scoped>
.notfound-wrap {
  display: grid;
  place-items: center;
}

.box {
  width: min(420px, 92vw);
  padding: 28px;
  text-align: center;
}

h1 {
  margin: 0;
  font-size: 64px;
}
</style>