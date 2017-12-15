function v = HELICS_VECTOR_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 15);
  end
  v = vInitialized;
end
