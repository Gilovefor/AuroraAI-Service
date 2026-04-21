<script setup>
import { reactive, ref } from 'vue'
import { useRouter } from 'vue-router'
import { login, register } from '../services/api'

const router = useRouter()
const mode = ref('login')
const loading = ref(false)
const message = ref('')

const form = reactive({
  username: '',
  password: ''
})

async function submit() {
  if (!form.username || !form.password || loading.value) return
  loading.value = true
  message.value = ''

  try {
    if (mode.value === 'login') {
      const { response, data } = await login({ username: form.username, password: form.password })
      if (response.status === 200 && data?.userId) {
        sessionStorage.setItem('userId', String(data.userId))
        router.push('/menu')
        return
      }
      message.value = response.status === 401 ? 'Invalid username or password' : 'Login failed'
    } else {
      const { response } = await register({ username: form.username, password: form.password })
      if (response.status === 200) {
        message.value = 'Registration succeeded, please login'
        mode.value = 'login'
        return
      }
      message.value = response.status === 409 ? 'User already exists' : 'Registration failed'
    }
  } catch {
    message.value = 'Request failed, please check network'
  } finally {
    loading.value = false
  }
}
</script>

<template>
  <main class="page-shell entry-wrap">
    <section class="card entry-card">
      <h1>AuroraAI Service</h1>
      <p class="muted">Fast and focused AI service experience</p>

      <div class="tabs">
        <button class="button secondary" :class="{ active: mode === 'login' }" @click="mode = 'login'">Login</button>
        <button class="button secondary" :class="{ active: mode === 'register' }" @click="mode = 'register'">Register</button>
      </div>

      <form class="form" @submit.prevent="submit">
        <input v-model.trim="form.username" class="input" placeholder="Username" />
        <input v-model.trim="form.password" class="input" type="password" placeholder="Password" />
        <button class="button" type="submit" :disabled="loading">
          {{ loading ? 'Processing...' : mode === 'login' ? 'Login' : 'Register' }}
        </button>
      </form>

      <p v-if="message" class="tip">{{ message }}</p>
    </section>
  </main>
</template>

<style scoped>
.entry-wrap {
  display: grid;
  place-items: center;
}

.entry-card {
  width: min(420px, 92vw);
  padding: 28px;
}

h1 {
  margin: 0;
  font-size: 28px;
}

.tabs {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
  margin: 18px 0;
}

.tabs .active {
  border-color: rgba(125, 162, 255, 0.7);
  background: rgba(125, 162, 255, 0.14);
}

.form {
  display: grid;
  gap: 12px;
}

.tip {
  margin-top: 12px;
  color: #ffd5db;
}
</style>