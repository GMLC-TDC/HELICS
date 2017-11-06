function v = HELICS_COMPLEX_TYPE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 17);
  end
  v = vInitialized;
end
