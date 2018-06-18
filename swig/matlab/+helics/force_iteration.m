function v = force_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176314);
  end
  v = vInitialized;
end
