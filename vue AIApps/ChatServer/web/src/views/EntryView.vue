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
    <section class="entry-card">
      <h1>AuroraAI Service</h1>

      <div class="tabs">
        <button class="tab-btn" :class="{ active: mode === 'login' }" @click="mode = 'login'">Login</button>
        <button class="tab-btn" :class="{ active: mode === 'register' }" @click="mode = 'register'">Register</button>
      </div>

      <form class="form" @submit.prevent="submit">
        <input v-model.trim="form.username" class="input mono-input" placeholder="Username" />
        <input v-model.trim="form.password" class="input mono-input" type="password" placeholder="Password" />
        <button class="submit-btn" type="submit" :disabled="loading">
          {{ loading ? 'Processing...' : mode === 'login' ? 'Login' : 'Register' }}
        </button>
      </form>

      <p v-if="message" class="tip">{{ message }}</p>
    </section>
  </main>
</template>

<style scoped>
.entry-wrap {
  min-height: 100vh;
  display: flex;
  justify-content: center;
  align-items: flex-start;
  padding-top: 12vh;
  background:
    linear-gradient(rgba(18, 18, 18, 0.52), rgba(18, 18, 18, 0.52)),
    url('/assets/entry_pic.jpg') center center / cover no-repeat;
}

.entry-card {
  width: min(420px, 92vw);
  padding: 26px 24px 22px;
  border: 1px solid rgba(255, 255, 255, 0.26);
  border-radius: 14px;
  background: rgba(25, 25, 25, 0.68);
  backdrop-filter: blur(4px);
  box-shadow: 0 18px 40px rgba(0, 0, 0, 0.36);
  color: #f2f2f2;
}

h1 {
  margin: 0;
  font-size: 26px;
  font-weight: 600;
  color: #ffffff;
}

.tabs {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
  margin: 18px 0 14px;
}

.tab-btn {
  height: 38px;
  border: 1px solid rgba(255, 255, 255, 0.24);
  border-radius: 10px;
  background: rgba(255, 255, 255, 0.06);
  color: #e7e7e7;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.18s ease;
}

.tab-btn:hover {
  background: rgba(255, 255, 255, 0.12);
}

.tab-btn.active {
  background: rgba(255, 255, 255, 0.18);
  border-color: rgba(255, 255, 255, 0.5);
  color: #ffffff;
}

.form {
  display: grid;
  gap: 10px;
}

.mono-input {
  border: 1px solid rgba(255, 255, 255, 0.28);
  background: rgba(255, 255, 255, 0.08);
  color: #ffffff;
}

.mono-input::placeholder {
  color: #c8c8c8;
}

.mono-input:focus {
  border-color: rgba(255, 255, 255, 0.62);
}

.submit-btn {
  margin-top: 2px;
  height: 40px;
  border: 1px solid rgba(255, 255, 255, 0.34);
  border-radius: 10px;
  background: #efefef;
  color: #141414;
  font-weight: 700;
  cursor: pointer;
  transition: all 0.18s ease;
}

.submit-btn:hover {
  background: #ffffff;
}

.submit-btn:disabled {
  opacity: 0.65;
  cursor: not-allowed;
}

.tip {
  margin-top: 10px;
  color: #ffd7d7;
  font-size: 13px;
}
</style>