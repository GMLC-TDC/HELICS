function v = force_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 10);
  end
  v = vInitialized;
end
