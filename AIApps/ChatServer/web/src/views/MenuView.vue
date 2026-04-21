<script setup>
import { useRouter } from 'vue-router'
import { logout } from '../services/api'

const router = useRouter()

async function doLogout() {
  const userId = sessionStorage.getItem('userId')
  try {
    await logout({ userId, type: 'manual', gameType: 0 })
  } finally {
    sessionStorage.clear()
    router.push('/')
  }
}
</script>

<template>
  <main class="page-shell menu-wrap">
    <section class="card menu-card">
      <h1>AI Features</h1>
      <p class="muted">Choose a module</p>

      <div class="grid">
        <button class="button tile" @click="router.push('/chat')">AI Chat</button>
        <button class="button tile" @click="router.push('/upload')">Image Recognition</button>
      </div>

      <button class="button danger" @click="doLogout">Logout</button>
    </section>
  </main>
</template>

<style scoped>
.menu-wrap {
  display: grid;
  place-items: center;
}

.menu-card {
  width: min(640px, 92vw);
  padding: 28px;
}

.grid {
  display: grid;
  grid-template-columns: repeat(2, minmax(0, 1fr));
  gap: 14px;
  margin: 18px 0 22px;
}

.tile {
  min-height: 100px;
  font-size: 16px;
}

@media (max-width: 640px) {
  .grid {
    grid-template-columns: 1fr;
  }
}
</style>