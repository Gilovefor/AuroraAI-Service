<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { uploadImage } from '../services/api'

const router = useRouter()
const fileRef = ref(null)
const records = ref([])
const loading = ref(false)

function push(role, text, image = '') {
  records.value.push({ role, text, image, id: Date.now() + Math.random() })
}

async function submit() {
  const file = fileRef.value?.files?.[0]
  if (!file || loading.value) return

  const reader = new FileReader()
  loading.value = true

  reader.onload = async (event) => {
    const base64Url = event.target?.result
    const base64Data = String(base64Url).split(',')[1] || ''

    push('user', `Uploaded image: ${file.name}`, String(base64Url))

    try {
      const { response, data } = await uploadImage({ filename: file.name, image: base64Data })
      if (!response.ok) throw new Error('Upload failed')
      push('assistant', `Result: ${data.class_name}, confidence ${Math.round(data.confidence * 100)}%`)
    } catch (err) {
      push('assistant', `Request failed: ${err.message}`)
    } finally {
      loading.value = false
      if (fileRef.value) fileRef.value.value = ''
    }
  }

  reader.readAsDataURL(file)
}
</script>

<template>
  <main class="page-shell upload-wrap">
    <section class="card upload-card">
      <header>
        <button class="button secondary" @click="router.push('/menu')">Back</button>
        <h2>Image Recognition</h2>
      </header>

      <div class="messages">
        <div v-for="item in records" :key="item.id" class="msg" :class="item.role">
          <strong>{{ item.role === 'user' ? 'Me' : 'AI' }}</strong>
          <p>{{ item.text }}</p>
          <img v-if="item.image" :src="item.image" alt="upload" />
        </div>
      </div>

      <form class="upload-form" @submit.prevent="submit">
        <input ref="fileRef" class="input" type="file" accept="image/*" required />
        <button class="button" type="submit" :disabled="loading">{{ loading ? 'Recognizing...' : 'Upload Image' }}</button>
      </form>
    </section>
  </main>
</template>

<style scoped>
.upload-wrap {
  display: grid;
  place-items: center;
}

.upload-card {
  width: min(980px, 94vw);
  min-height: 82vh;
  padding: 18px;
  display: grid;
  grid-template-rows: auto 1fr auto;
  gap: 12px;
}

header {
  display: flex;
  align-items: center;
  gap: 12px;
}

.messages {
  border: 1px solid var(--border);
  border-radius: 14px;
  padding: 12px;
  overflow: auto;
  background: rgba(0, 0, 0, 0.16);
}

.msg {
  max-width: 74%;
  margin-bottom: 10px;
  padding: 10px 12px;
  border-radius: 12px;
}

.msg.user {
  margin-left: auto;
  background: rgba(125, 162, 255, 0.22);
}

.msg.assistant {
  background: rgba(255, 255, 255, 0.08);
}

.msg p {
  margin: 8px 0 0;
}

.msg img {
  margin-top: 8px;
  max-width: 220px;
  border-radius: 8px;
}

.upload-form {
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 10px;
}

@media (max-width: 640px) {
  .upload-form {
    grid-template-columns: 1fr;
  }
}
</style>