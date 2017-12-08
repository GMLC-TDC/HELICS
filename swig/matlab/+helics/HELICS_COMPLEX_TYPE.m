function v = HELICS_COMPLEX_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 14);
  end
  v = vInitialized;
end
